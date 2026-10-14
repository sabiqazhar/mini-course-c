#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generator HTML interaktif untuk kursus "Low-Level C" (12 minggu).

Pakai:
    python3 build_html.py <folder_sumber> <file_output.html>

Folder sumber = repo kursus yang berisi README.md, setup.md, plan.md,
glossary.md, references.md, modules/week-NN-*.md, labs/week-NN/, solutions/week-NN/.

Hasilnya SATU file HTML mandiri (offline): sidebar, tab per minggu,
pencarian full-text, checklist interaktif (localStorage), dark mode,
syntax highlighting via Pygments.
"""

import html
import json
import re
import sys
from pathlib import Path

import markdown as md
from pygments import highlight
from pygments.formatters import HtmlFormatter
from pygments.lexers import get_lexer_by_name
from pygments.lexers.special import TextLexer

# ---------------------------------------------------------------- utils


def strip_tags(s: str) -> str:
    # span = token hasil Pygments: dibuang tanpa spasi supaya kode tetap utuh
    s = re.sub(r"</?span[^>]*>", "", s)
    s = re.sub(r"<[^>]+>", " ", s)
    s = html.unescape(s)
    return re.sub(r"\s+", " ", s).strip()


def slug(value: str, separator: str = "-") -> str:
    s = re.sub(r"[^a-z0-9]+", separator, value.lower())
    return s.strip(separator) or "bagian"


CODE_LEXER = {
    "c": "c",
    "h": "c",
    "cpp": "cpp",
    "sh": "bash",
    "bash": "bash",
    "console": "text",
    "text": "text",
    "makefile": "makefile",
    "make": "makefile",
    "asm": "nasm",
    "py": "python",
}


def pygmentize(code: str, lang: str) -> str:
    lang = (lang or "text").lower()
    name = CODE_LEXER.get(lang, "text")
    try:
        lexer = get_lexer_by_name(name, stripall=False, ensurenl=False)
    except Exception:
        lexer = TextLexer(stripnl=False)
    return highlight(code, lexer, HtmlFormatter(nowrap=True)).rstrip("\n")


CODE_RE = re.compile(
    r'<pre><code(?: class="language-([^"]+)")?>(.*?)</code></pre>', re.S
)


def highlight_blocks(fragment: str) -> str:
    """Ganti <pre><code class="language-x"> di hasil markdown dengan Pygments."""

    def repl(m):
        lang = m.group(1) or "text"
        code = html.unescape(m.group(2))
        body = pygmentize(code, lang)
        return (
            '<div class="codewrap"><button class="copybtn" type="button">'
            "Salin</button>"
            f'<pre class="code" data-lang="{html.escape(lang)}"><code>{body}</code></pre>'
            "</div>"
        )

    return CODE_RE.sub(repl, fragment)


MD = md.Markdown(
    extensions=["extra", "toc", "sane_lists"],
    extension_configs={"toc": {"permalink": False, "slugify": slug}},
)


def md_to_html(text: str, id_prefix: str = "") -> str:
    MD.reset()
    out = MD.convert(text)
    if id_prefix:
        out = re.sub(r'(<h[1-6][^>]*\sid=")', rf'\1{id_prefix}-', out)
    out = highlight_blocks(out)
    return out


def code_card(code: str, lang: str, filename: str = "", cmd: str = "") -> str:
    body = pygmentize(code, lang)
    parts = ['<div class="codefile">']
    if filename:
        parts.append(f'<div class="codefile-head"><span class="fname">{html.escape(filename)}</span>')
        if cmd:
            parts.append(f'<code class="cmd">{html.escape(cmd)}</code>')
        parts.append("</div>")
    parts.append(
        '<div class="codewrap"><button class="copybtn" type="button">Salin</button>'
        f'<pre class="code" data-lang="{html.escape(lang)}"><code>{body}</code></pre></div>'
    )
    parts.append("</div>")
    return "".join(parts)


def compile_cmd(src: str, filename: str) -> str:
    stem = Path(filename).stem
    flags = "-std=c11 -Wall -Wextra -Wpedantic -g -O0"
    libs = ""
    if "pthread" in src:
        libs = " -pthread"
    return f"gcc {flags} -o {stem} {filename}{libs}"


HEAD_RE = re.compile(r"<h([234])[^>]*>(.*?)</h\1>", re.S)
SUM_RE = re.compile(r"<summary[^>]*>(.*?)</summary>", re.S)


def chunk_html(fragment: str, page: str, tab: str, index: list):
    """Pecah HTML per heading (dan per file lab) untuk indeks pencarian."""
    fragment = re.sub(r'<button class="copybtn".*?</button>', " ", fragment, flags=re.S)
    marks = []
    for m in HEAD_RE.finditer(fragment):
        marks.append((m.start(), m.end(), m.group(2), m.group(0)))
    for m in SUM_RE.finditer(fragment):
        marks.append((m.start(), m.end(), m.group(1), m.group(0)))
    marks.sort(key=lambda t: t[0])

    def add(hid, title, body):
        text = strip_tags(body)
        if len(text) < 25:
            return
        index.append(
            {
                "p": page,
                "t": tab,
                "h": hid or "",
                "ti": strip_tags(title)[:90] or "Awal",
                "x": text[:4000],
            }
        )

    pre = fragment[: marks[0][0]] if marks else fragment
    if strip_tags(pre):
        add("", "Pengantar", pre)
    for i, (start, end, title, full) in enumerate(marks):
        stop = marks[i + 1][0] if i + 1 < len(marks) else len(fragment)
        idm = re.search(r'id="([^"]+)"', full)
        add(idm.group(1) if idm else "", title, fragment[end:stop])


# ---------------------------------------------------------------- parsing modul


def split_sections(text: str):
    """Pisah markdown per '## ' (aman terhadap fence ```)."""
    lines = text.split("\n")
    head, sections = [], []
    cur_title, cur, in_fence = None, [], False
    for ln in lines:
        if ln.lstrip().startswith("```"):
            in_fence = not in_fence
        if not in_fence and ln.startswith("## "):
            if cur_title is not None:
                sections.append((cur_title, "\n".join(cur)))
            cur_title, cur = ln[3:].strip(), []
            continue
        if cur_title is None:
            head.append(ln)
        else:
            cur.append(ln)
    if cur_title is not None:
        sections.append((cur_title, "\n".join(cur)))
    return "\n".join(head), sections


def sec_num(title: str):
    m = re.match(r"^(\d+)\.\s*(.*)$", title)
    return (int(m.group(1)), m.group(2).strip()) if m else (None, title)


CHECKLIST_RE = re.compile(r"^\s*[-*]\s+(?:\[[ xX]\]\s*)?(.*)$")


def checklist_items(body: str):
    items, rest = [], []
    for ln in body.split("\n"):
        m = CHECKLIST_RE.match(ln)
        if m and not ln.lstrip().startswith("|"):
            items.append(m.group(1).strip())
        else:
            rest.append(ln)
    return items, "\n".join(rest)


# ---------------------------------------------------------------- build


def build(src: Path):
    pages = []          # list of dict
    index = []          # search index

    def add_page(pid, title, eyebrow, html_body, group):
        pages.append(
            {
                "id": pid,
                "title": title,
                "eyebrow": eyebrow,
                "html": html_body,
                "group": group,
                "tabs": None,
            }
        )
        chunk_html(html_body, pid, "", index)

    # ---- halaman statis
    statics = [
        ("home", "Beranda", "Mulai di sini", "README.md", "start"),
        ("setup", "Setup Toolchain", "Mulai di sini", "setup.md", "start"),
        ("plan", "Rencana Belajar 12 Minggu", "Mulai di sini", "plan.md", "start"),
        ("glossary", "Glosarium", "Rujukan", "glossary.md", "ref"),
        ("references", "Referensi & Sumber", "Rujukan", "references.md", "ref"),
    ]
    for pid, title, eyebrow, fname, group in statics:
        f = src / fname
        if not f.exists():
            continue
        body = md_to_html(f.read_text(encoding="utf-8"), pid)
        add_page(pid, title, eyebrow, body, group)

    # ---- minggu 1..12
    weeks = []
    for mod in sorted(src.glob("modules/week-*.md")):
        m = re.match(r"week-(\d+)-", mod.name)
        if not m:
            continue
        wnum = int(m.group(1))
        wid = f"w{wnum:02d}"
        raw = mod.read_text(encoding="utf-8")

        h1 = re.search(r"^#\s+(.*)$", raw, re.M)
        full_title = h1.group(1).strip() if h1 else mod.stem
        short = full_title.split("—", 1)[-1].strip() if "—" in full_title else full_title

        head, sections = split_sections(raw)
        materi, latihan_md, check_md = [], None, None
        for title, body in sections:
            # Klasifikasi berbasis kata kunci (nomor bagian tidak konsisten:
            # minggu 11 cuma punya 11 bagian, jadi Latihan ada di §7).
            low = title.lower()
            if "checklist" in low:
                check_md = body
            elif "latihan" in low:
                latihan_md = body
            else:
                materi.append(f"## {title}\n{body}")

        # ---- tab Materi
        materi_html = md_to_html("\n".join(materi), wid)

        # ---- tab Lab
        lab_dir = src / "labs" / f"week-{wnum:02d}"
        lab_html = [
            '<p class="note">Semua file di bawah ini ada di folder '
            f"<code>labs/week-{wnum:02d}/</code>. Salin, kompilasi, lalu jalankan sendiri "
            "— jangan cuma dibaca.</p>"
        ]
        lab_files = []
        if lab_dir.is_dir():
            lab_files = sorted(
                p for p in lab_dir.iterdir()
                if p.is_file() and (p.suffix in {".c", ".h"} or p.name == "Makefile")
            )
        if not lab_files:
            lab_html.append("<p><em>Belum ada file lab untuk minggu ini.</em></p>")
        for p in lab_files:
            code = p.read_text(encoding="utf-8", errors="replace")
            lang = "makefile" if p.name == "Makefile" else ("c" if p.suffix == ".c" else "c")
            cmd = "" if p.name == "Makefile" else compile_cmd(code, p.name)
            broken = "rusak" in p.name or "broken" in p.name
            cls = " sol-block" if broken else ""
            lab_html.append(
                f'<details class="lab{cls}"><summary><span class="labname">'
                f'{html.escape(p.name)}</span>'
                + (' <span class="tag tag-broken">sengaja rusak</span>' if broken else "")
                + "</summary>"
            )
            lab_html.append('<div class="labbody">')
            lab_html.append(code_card(code, lang, p.name, cmd))
            lab_html.append("</div></details>")
        lab_html = "".join(lab_html)

        # ---- tab Latihan (+ kunci jawaban)
        lat_html = []
        if latihan_md:
            lat_html.append(md_to_html(latihan_md, wid + "-lat"))
        else:
            lat_html.append("<p><em>Tidak ada bagian latihan terpisah di modul minggu ini.</em></p>")

        sol_dir = src / "solutions" / f"week-{wnum:02d}"
        sol_files, sol_intro = [], ""
        if sol_dir.is_dir():
            sol_files = sorted(
                p for p in sol_dir.iterdir()
                if p.is_file() and (p.suffix in {".c", ".h"} or p.name == "Makefile")
            )
            readme = sol_dir / "README.md"
            if readme.exists():
                sol_intro = md_to_html(readme.read_text(encoding="utf-8"), wid + "-sol")

        if sol_files:
            lat_html.append(
                '<p class="note">Kunci jawaban ada di tab <strong>Solusi</strong> '
                "(atau folder "
                f"<code>solutions/week-{wnum:02d}/</code>). Coba kerjakan sendiri dulu, "
                "baru bandingkan.</p>"
            )

        # ---- tab Solusi
        sol_html = []
        if sol_intro:
            sol_html.append(sol_intro)
        if sol_files:
            if not sol_intro:
                sol_html.append(
                    '<p class="note">Kode di bawah adalah <em>satu</em> cara menyelesaikan '
                    "latihan minggu ini. Bandingkan dengan jawabanmu sendiri.</p>"
                )
            for p in sol_files:
                code = p.read_text(encoding="utf-8", errors="replace")
                lang = "makefile" if p.name == "Makefile" else "c"
                cmd = "" if p.name == "Makefile" else compile_cmd(code, p.name)
                sol_html.append(
                    f'<details class="lab"><summary><span class="labname">'
                    f'{html.escape(p.name)}</span></summary><div class="labbody">'
                )
                sol_html.append(code_card(code, lang, p.name, cmd))
                sol_html.append("</div></details>")
        if not sol_files and not sol_intro:
            sol_html.append(
                "<p><em>Belum ada kunci jawaban untuk minggu ini.</em></p>"
            )
        sol_html = "".join(sol_html)

        # ---- tab Checklist (interaktif)
        items, rest = ([], "")
        if check_md:
            items, rest = checklist_items(check_md)
        ck = ['<div class="checklist" data-week="%d">' % wnum]
        if rest.strip():
            ck.append(md_to_html(rest, wid + "-ck"))
        if items:
            ck.append('<ul class="checks">')
            for i, it in enumerate(items):
                txt = md_to_html_inline(it)
                ck.append(
                    f'<li><label><input type="checkbox" data-check="{wid}-{i}">'
                    f"<span>{txt}</span></label></li>"
                )
            ck.append("</ul>")
            ck.append(
                f'<p class="checkmeta"><span class="checkcount" data-count="{wid}">'
                f"0 / {len(items)}</span> tercentang</p>"
            )
        else:
            ck.append("<p><em>Tidak ada checklist di modul minggu ini.</em></p>")
        ck.append(
            f'<div class="weekdone"><button class="donebtn" data-done="{wid}">'
            "Tandai minggu ini selesai</button>"
            f'<span class="donestate" data-donestate="{wid}"></span></div>'
        )
        ck.append("</div>")
        check_html = "".join(ck)

        tabdefs = [
            ("materi", "Materi", materi_html, ""),
            ("lab", "Lab", lab_html, str(len(lab_files))),
            ("latihan", "Latihan", "".join(lat_html), ""),
            ("solusi", "Solusi", sol_html, str(len(sol_files))),
            ("checklist", "Checklist", check_html, f"0/{len(items)}"),
        ]
        for tname, _, thtml, _badge in tabdefs:
            chunk_html(thtml, wid, tname, index)

        weeks.append(
            {
                "id": wid,
                "num": wnum,
                "title": short,
                "full": full_title,
                "tabs": [
                    {"id": t, "label": l, "badge": b, "html": h}
                    for t, l, h, b in tabdefs
                ],
            }
        )

    return pages, weeks, index


INLINE_MD = md.Markdown(extensions=["extra"])


def md_to_html_inline(text: str) -> str:
    INLINE_MD.reset()
    out = INLINE_MD.convert(text)
    out = re.sub(r"^<p>|</p>$", "", out.strip())
    return highlight_blocks(out)


# ---------------------------------------------------------------- shell

CSS = """
:root{
  --bg:#fbfbfa; --panel:#ffffff; --fg:#1c1c1a; --muted:#5f5f5a;
  --border:#e3e2de; --accent:#b45309; --accent-soft:#fdf3e7;
  --sidebar:#f4f3f0; --shadow:0 1px 2px rgba(0,0,0,.05);
  --code-bg:#f6f5f2;
}
html.dark{
  --bg:#16171a; --panel:#1c1e22; --fg:#e6e6e3; --muted:#9a9a95;
  --border:#2e3136; --accent:#e0a458; --accent-soft:#2a2419;
  --sidebar:#141519; --shadow:0 1px 2px rgba(0,0,0,.4);
  --code-bg:#23262b;
}
*{box-sizing:border-box}
html,body{margin:0;padding:0}
body{
  background:var(--bg); color:var(--fg);
  font:16px/1.72 -apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,"Helvetica Neue",Arial,sans-serif;
  -webkit-font-smoothing:antialiased;
}
a{color:var(--accent);text-decoration:none}
a:hover{text-decoration:underline}

/* layout */
#app{display:flex;min-height:100vh}
#sidebar{
  width:290px;flex:0 0 290px;background:var(--sidebar);
  border-right:1px solid var(--border);
  height:100vh;position:sticky;top:0;overflow-y:auto;padding:18px 14px 60px;
}
#main{flex:1;min-width:0;padding:0 0 120px}
.wrap{max-width:820px;margin:0 auto;padding:26px 30px}

/* sidebar */
.brand{display:flex;align-items:center;gap:10px;margin-bottom:14px}
.brand b{font-size:15px;letter-spacing:.2px}
.brand span{display:block;font-size:11.5px;color:var(--muted)}
.side-group{margin:16px 0 6px;font-size:11px;font-weight:700;letter-spacing:.9px;
  text-transform:uppercase;color:var(--muted);padding:0 8px}
.navlink{
  display:flex;align-items:center;gap:8px;width:100%;text-align:left;
  background:none;border:0;color:var(--fg);font:inherit;font-size:14px;
  padding:7px 10px;border-radius:8px;cursor:pointer;line-height:1.35;
}
.navlink:hover{background:var(--panel);text-decoration:none}
.navlink.active{background:var(--accent-soft);color:var(--accent);font-weight:600}
.navnum{
  flex:0 0 24px;height:22px;border-radius:6px;background:var(--panel);
  border:1px solid var(--border);font-size:11.5px;display:flex;align-items:center;
  justify-content:center;color:var(--muted);
}
.navlink.active .navnum{background:var(--accent);color:#fff;border-color:transparent}
.navtick{margin-left:auto;font-size:12px;color:var(--accent)}
.progressbox{
  margin:4px 0 14px;padding:10px 12px;border:1px solid var(--border);
  border-radius:10px;background:var(--panel);box-shadow:var(--shadow)
}
.progressbox .lbl{display:flex;justify-content:space-between;font-size:11.5px;color:var(--muted);margin-bottom:6px}
.bar{height:6px;border-radius:99px;background:var(--border);overflow:hidden}
.bar>i{display:block;height:100%;background:var(--accent);width:0;transition:width .25s}

/* topbar */
#topbar{
  position:sticky;top:0;z-index:20;background:var(--bg);
  border-bottom:1px solid var(--border);padding:8px 16px;
  display:flex;align-items:center;gap:10px
}
#search{flex:1;max-width:520px;padding:8px 12px;border-radius:9px;
  border:1px solid var(--border);background:var(--panel);color:var(--fg);font:inherit;font-size:14px}
.iconbtn{
  border:1px solid var(--border);background:var(--panel);color:var(--fg);
  border-radius:9px;padding:7px 11px;cursor:pointer;font:inherit;font-size:13px
}
.iconbtn:hover{border-color:var(--accent)}
#menubtn{display:none}

/* pages */
.page{display:none}
.page.active{display:block}
.eyebrow{font-size:11.5px;letter-spacing:1px;text-transform:uppercase;color:var(--muted);font-weight:700}
.page-title{font-size:27px;line-height:1.25;margin:.25em 0 .15em}

/* tabs */
.tabbar{display:flex;gap:6px;flex-wrap:wrap;margin:18px 0 20px;border-bottom:1px solid var(--border);padding-bottom:2px}
.tabbar button{
  background:none;border:0;border-bottom:2px solid transparent;color:var(--muted);
  font:inherit;font-size:14px;padding:8px 12px;cursor:pointer;border-radius:6px 6px 0 0
}
.tabbar button:hover{color:var(--fg);background:var(--panel)}
.tabbar button.active{color:var(--accent);border-bottom-color:var(--accent);font-weight:600}
.tabbar .badge{
  font-size:11px;background:var(--panel);border:1px solid var(--border);
  border-radius:99px;padding:1px 6px;margin-left:5px;color:var(--muted)
}
.tabpanel{display:none}
.tabpanel.active{display:block}

/* content */
.wrap h1,.wrap h2,.wrap h3,.wrap h4{line-height:1.3}
.wrap h2{font-size:21px;margin:2em 0 .6em;padding-top:.4em;border-top:1px solid var(--border)}
.wrap h3{font-size:17px;margin:1.7em 0 .5em}
.wrap h4{font-size:15px;margin:1.4em 0 .4em}
.wrap p{margin:.85em 0}
.wrap ul,.wrap ol{padding-left:1.35em}
.wrap li{margin:.35em 0}
.wrap blockquote{
  margin:1.1em 0;padding:.6em 1em;border-left:3px solid var(--accent);
  background:var(--accent-soft);border-radius:0 8px 8px 0;color:var(--fg)
}
.wrap hr{border:0;border-top:1px solid var(--border);margin:2em 0}
.wrap table{border-collapse:collapse;width:100%;font-size:14px;margin:1.2em 0;display:block;overflow-x:auto}
.wrap th,.wrap td{border:1px solid var(--border);padding:7px 10px;text-align:left;vertical-align:top}
.wrap th{background:var(--panel);font-weight:600}
.wrap :not(pre)>code{
  background:var(--code-bg);border:1px solid var(--border);border-radius:5px;
  padding:.1em .35em;font-size:.88em;font-family:ui-monospace,SFMono-Regular,Menlo,Consolas,monospace
}
.note{
  background:var(--panel);border:1px solid var(--border);border-left:3px solid var(--accent);
  border-radius:8px;padding:10px 14px;font-size:14.5px;box-shadow:var(--shadow)
}

/* code */
.codewrap{position:relative;margin:1.1em 0}
pre.code{
  background:var(--code-bg);border:1px solid var(--border);border-radius:10px;
  padding:14px 16px;overflow-x:auto;font-size:13.2px;line-height:1.62;
  font-family:ui-monospace,SFMono-Regular,Menlo,Consolas,monospace;
  box-shadow:var(--shadow)
}
.copybtn{
  position:absolute;top:8px;right:8px;z-index:2;font:inherit;font-size:11.5px;
  border:1px solid var(--border);background:var(--panel);color:var(--muted);
  border-radius:6px;padding:3px 8px;cursor:pointer;opacity:0;transition:opacity .15s
}
.codewrap:hover .copybtn{opacity:1}
.copybtn.done{color:var(--accent);border-color:var(--accent)}
.codefile{margin:1em 0}
.codefile-head{
  display:flex;flex-wrap:wrap;gap:8px;align-items:baseline;
  font-size:12.5px;color:var(--muted);margin-bottom:-6px
}
.codefile-head .fname{font-family:ui-monospace,Menlo,Consolas,monospace;color:var(--fg);font-weight:600}
.codefile-head .cmd{font-size:11.5px;opacity:.85}

/* details (lab / solusi) */
details.lab,details.sol{
  border:1px solid var(--border);border-radius:10px;margin:.8em 0;background:var(--panel);box-shadow:var(--shadow)
}
details.lab>summary,details.sol>summary{
  cursor:pointer;padding:10px 14px;font-size:14.5px;list-style:none;display:flex;align-items:center;gap:8px
}
details.lab>summary::-webkit-details-marker,details.sol>summary::-webkit-details-marker{display:none}
details.lab>summary::before,details.sol>summary::before{content:"\\25B8";color:var(--muted);transition:transform .15s}
details[open]>summary::before{transform:rotate(90deg)}
.labname{font-family:ui-monospace,Menlo,Consolas,monospace;font-weight:600}
.labbody{padding:0 14px 6px}
.tag{font-size:10.5px;border-radius:99px;padding:2px 7px;border:1px solid var(--border);color:var(--muted)}
.tag-broken{background:#fdecec;border-color:#f3c9c9;color:#a33}
html.dark .tag-broken{background:#3a1f1f;border-color:#5a2c2c;color:#e59a9a}
details.lab .codewrap{margin:.6em 0}

/* checklist */
.checks{list-style:none;padding:0;margin:1em 0}
.checks li{margin:.45em 0}
.checks label{display:flex;gap:10px;align-items:flex-start;cursor:pointer;font-size:15px;line-height:1.55}
.checks input{width:17px;height:17px;margin-top:4px;accent-color:var(--accent);flex:0 0 17px;cursor:pointer}
.checks input:checked + span{color:var(--muted);text-decoration:line-through;text-decoration-color:var(--border)}
.checkmeta{font-size:13px;color:var(--muted)}
.weekdone{margin-top:18px;display:flex;align-items:center;gap:12px;flex-wrap:wrap}
.donebtn{
  border:1px solid var(--accent);background:var(--accent-soft);color:var(--accent);
  border-radius:9px;padding:9px 15px;font:inherit;font-size:14px;cursor:pointer
}
.donebtn.on{background:var(--accent);color:#fff}
.donestate{font-size:13px;color:var(--muted)}

/* pager */
.pager{display:flex;justify-content:space-between;gap:12px;margin:44px 0 10px;padding-top:18px;border-top:1px solid var(--border)}
.pager button{
  background:var(--panel);border:1px solid var(--border);border-radius:9px;
  padding:9px 14px;font:inherit;font-size:14px;cursor:pointer;color:var(--fg);box-shadow:var(--shadow)
}
.pager button:hover{border-color:var(--accent);color:var(--accent)}

/* search results */
#results{
  position:absolute;top:52px;left:16px;right:16px;max-width:620px;
  background:var(--panel);border:1px solid var(--border);border-radius:12px;
  box-shadow:0 10px 30px rgba(0,0,0,.16);max-height:62vh;overflow-y:auto;display:none;z-index:40
}
#results.on{display:block}
#results .r{padding:10px 14px;border-bottom:1px solid var(--border);cursor:pointer}
#results .r:hover,#results .r.sel{background:var(--accent-soft)}
#results .rt{font-size:13.5px;font-weight:600}
#results .rp{font-size:11.5px;color:var(--muted);margin-bottom:2px}
#results .rx{font-size:12.5px;color:var(--muted);line-height:1.5;
  display:-webkit-box;-webkit-line-clamp:2;-webkit-box-orient:vertical;overflow:hidden}
#results .empty{padding:14px;color:var(--muted);font-size:13.5px}
mark{background:#ffe9a8;color:inherit;border-radius:2px}
html.dark mark{background:#5a4a1e;color:#ffe9a8}

/* highlight target */
.hit{animation:flash 1.4s ease}
@keyframes flash{0%{background:var(--accent-soft)}100%{background:transparent}}

@media (max-width:820px){
  #sidebar{position:fixed;left:0;top:0;transform:translateX(-100%);transition:transform .2s;z-index:60;width:280px}
  #sidebar.open{transform:none}
  #menubtn{display:inline-block}
  .wrap{padding:18px 16px}
  #main{padding-bottom:80px}
}
@media print{
  #sidebar,#topbar,.pager,.copybtn{display:none!important}
  .page{display:block!important}
  .tabpanel{display:block!important}
}
"""

JS = r"""
(function(){
  var KEY='llc.v1';
  var st={page:'home',tabs:{},checks:{},done:{},theme:'light'};
  try{ var s=JSON.parse(localStorage.getItem(KEY)||'{}'); for(var k in s){st[k]=s[k];} }catch(e){}
  function save(){ try{ localStorage.setItem(KEY,JSON.stringify(st)); }catch(e){} }

  var DATA=JSON.parse(document.getElementById('course-data').textContent);
  var INDEX=JSON.parse(document.getElementById('search-index').textContent);

  /* ---- tema ---- */
  function setTheme(t){
    st.theme=t; document.documentElement.classList.toggle('dark', t==='dark');
    document.getElementById('themebtn').textContent = t==='dark' ? 'Terang' : 'Gelap';
    save();
  }
  setTheme(st.theme||'light');
  document.getElementById('themebtn').onclick=function(){
    setTheme(st.theme==='dark'?'light':'dark');
  };

  /* ---- sidebar ---- */
  var sb=document.getElementById('navlist');
  var order=[];
  DATA.pages.forEach(function(p){ order.push({id:p.id,tab:''}); });
  DATA.weeks.forEach(function(w){ order.push({id:w.id,tab:'materi'}); });

  function buildNav(){
    var h='';
    h+='<div class="progressbox"><div class="lbl"><span>Progres belajar</span>'
      +'<span id="pct">0%</span></div><div class="bar"><i id="pbar"></i></div>'
      +'<div class="lbl" style="margin:7px 0 0"><span id="psum">0/12 minggu</span>'
      +'<span id="csum"></span></div></div>';
    h+='<div class="side-group">Mulai di sini</div>';
    DATA.pages.forEach(function(p){
      if(p.group!=='start') return;
      h+='<button class="navlink" data-go="'+p.id+'" data-tab="">'
        +'<span class="navnum">&sect;</span><span>'+p.title+'</span></button>';
    });
    h+='<div class="side-group">Minggu 1 – 12</div>';
    DATA.weeks.forEach(function(w){
      h+='<button class="navlink" data-go="'+w.id+'" data-tab="materi">'
        +'<span class="navnum">'+w.num+'</span><span>'+esc(w.title)+'</span>'
        +'<span class="navtick" data-tick="'+w.id+'"></span></button>';
    });
    h+='<div class="side-group">Rujukan</div>';
    DATA.pages.forEach(function(p){
      if(p.group!=='ref') return;
      h+='<button class="navlink" data-go="'+p.id+'" data-tab="">'
        +'<span class="navnum">&sect;</span><span>'+p.title+'</span></button>';
    });
    sb.innerHTML=h;
  }
  function esc(s){ return (s||'').replace(/[&<>"]/g,function(c){
    return {'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]; }); }

  /* ---- render halaman ---- */
  var host=document.getElementById('pages');
  function renderPages(){
    var h='';
    DATA.pages.forEach(function(p){
      h+='<section class="page" data-page="'+p.id+'"><div class="wrap">'
        +'<div class="eyebrow">'+esc(p.eyebrow)+'</div>'
        +'<h1 class="page-title">'+esc(p.title)+'</h1>'
        +p.html+'</div></section>';
    });
    DATA.weeks.forEach(function(w){
      h+='<section class="page" data-page="'+w.id+'"><div class="wrap">'
        +'<div class="eyebrow">Minggu '+w.num+'</div>'
        +'<h1 class="page-title">'+esc(w.full)+'</h1>'
        +'<div class="tabbar">';
      w.tabs.forEach(function(t,i){
        h+='<button data-tab="'+t.id+'"'+(i===0?' class="active"':'')+'>'+t.label
          +(t.badge?'<span class="badge" data-badge="'+w.id+'-'+t.id+'">'+t.badge+'</span>':'')
          +'</button>';
      });
      h+='</div>';
      w.tabs.forEach(function(t,i){
        h+='<div class="tabpanel'+(i===0?' active':'')+'" data-tab="'+t.id+'">'+t.html+'</div>';
      });
      h+=pager(w.id)+'</div></section>';
    });
    host.innerHTML=h;
  }
  function pager(id){
    var i=order.findIndex(function(o){return o.id===id;});
    var prev=order[i-1], next=order[i+1];
    var h='<div class="pager">';
    h+= prev? '<button data-go="'+prev.id+'" data-tab="'+prev.tab+'">&larr; '
        +(prev.tab?'Minggu '+DATA.weeks.find(function(w){return w.id===prev.id;}).num
          :DATA.pages.find(function(p){return p.id===prev.id;}).title)+'</button>' : '<span></span>';
    h+= next? '<button data-go="'+next.id+'" data-tab="'+next.tab+'">'
        +(next.tab?'Minggu '+DATA.weeks.find(function(w){return w.id===next.id;}).num
          :DATA.pages.find(function(p){return p.id===next.id;}).title)+' &rarr;</button>' : '<span></span>';
    h+='</div>';
    return h;
  }

  /* ---- navigasi ---- */
  function go(id,tab,hash){
    var sec=host.querySelector('[data-page="'+id+'"]');
    if(!sec) return;
    host.querySelectorAll('.page').forEach(function(p){p.classList.remove('active');});
    sec.classList.add('active');
    if(tab){ selectTab(sec,tab); }
    st.page=id; save();
    document.querySelectorAll('#navlist .navlink').forEach(function(b){
      b.classList.toggle('active', b.dataset.go===id);
    });
    document.getElementById('sidebar').classList.remove('open');
    window.scrollTo(0,0);
    if(hash){
      var el=sec.querySelector('#'+CSS.escape(hash));
      if(el){ el.scrollIntoView({block:'start'}); el.classList.add('hit');
        setTimeout(function(){el.classList.remove('hit');},1600); }
    }
  }
  function selectTab(sec,tab){
    sec.querySelectorAll('.tabbar button').forEach(function(b){
      b.classList.toggle('active', b.dataset.tab===tab);
    });
    sec.querySelectorAll('.tabpanel').forEach(function(p){
      p.classList.toggle('active', p.dataset.tab===tab);
    });
    if(tab) st.tabs[sec.dataset.page]=tab;
    save();
  }

  /* ---- checklist & progres ---- */
  function refreshProgress(){
    var totalChecks=0, doneChecks=0;
    document.querySelectorAll('.checklist').forEach(function(cl){
      var w=cl.dataset.week; totalChecks+=cl.querySelectorAll('input[data-check]').length;
    });
    for(var k in st.checks){ if(st.checks[k]) doneChecks++; }
    var weeksDone=0;
    DATA.weeks.forEach(function(w){ if(st.done[w.id]) weeksDone++; });
    var pct = totalChecks? Math.round(100*(doneChecks+weeksDone*2)/(totalChecks+DATA.weeks.length*2)) : 0;
    document.getElementById('pbar').style.width=pct+'%';
    document.getElementById('pct').textContent=pct+'%';
    document.getElementById('psum').textContent=weeksDone+'/12 minggu selesai';
    document.getElementById('csum').textContent=doneChecks+'/'+totalChecks+' checklist';

    document.querySelectorAll('.checklist').forEach(function(cl){
      var id='w'+('0'+cl.dataset.week).slice(-2);
      var ins=cl.querySelectorAll('input[data-check]');
      var n=0;
      ins.forEach(function(i){ if(st.checks[i.dataset.check]){i.checked=true;n++;} });
      var cnt=cl.querySelector('.checkcount'); if(cnt) cnt.textContent=n+' / '+ins.length;
      var badge=document.querySelector('[data-badge="'+id+'-checklist"]');
      if(badge) badge.textContent=n+'/'+ins.length;
    });
    DATA.weeks.forEach(function(w){
      var t=document.querySelector('[data-tick="'+w.id+'"]');
      if(t) t.textContent = st.done[w.id] ? '\u2713' : '';
      var b=document.querySelector('[data-donestate="'+w.id+'"]');
      if(b) b.textContent = st.done[w.id] ? 'Minggu ini ditandai selesai.' : '';
      var btn=document.querySelector('[data-done="'+w.id+'"]');
      if(btn) btn.classList.toggle('on', !!st.done[w.id]);
    });
  }

  /* ---- wire up ---- */
  buildNav(); renderPages();
  document.addEventListener('click',function(e){
    var g=e.target.closest('[data-go]');
    if(g){ go(g.dataset.go, g.dataset.tab||'', g.dataset.hash||''); return; }
  });
  host.addEventListener('click',function(e){
    var t=e.target.closest('.tabbar button');
    if(t){ selectTab(t.closest('.page'), t.dataset.tab); return; }
    var c=e.target.closest('.copybtn');
    if(c){
      var pre=c.parentElement.querySelector('pre');
      var txt=pre? pre.innerText : '';
      navigator.clipboard && navigator.clipboard.writeText(txt).then(function(){
        c.textContent='Tersalin'; c.classList.add('done');
        setTimeout(function(){c.textContent='Salin';c.classList.remove('done');},1200);
      });
      return;
    }
    var d=e.target.closest('.donebtn');
    if(d){ var id=d.dataset.done; st.done[id]=!st.done[id]; save(); refreshProgress(); return; }
  });
  host.addEventListener('change',function(e){
    var i=e.target.closest('input[data-check]');
    if(i){ st.checks[i.dataset.check]=i.checked; save(); refreshProgress(); }
  });

  /* ---- pencarian ---- */
  var inp=document.getElementById('search'), res=document.getElementById('results');
  var sel=0, cur=[];
  function search(q){
    q=q.trim().toLowerCase();
    if(q.length<2){ res.classList.remove('on'); return; }
    var terms=q.split(/\s+/), out=[];
    INDEX.forEach(function(e){
      var t=e.x.toLowerCase(), ti=e.ti.toLowerCase(), sc=0;
      terms.forEach(function(w){
        var a=(t.split(w).length-1); var b=(ti.split(w).length-1)*4;
        if(a+b===0){ sc=-999; } else { sc+=a+b; }
      });
      if(sc>0) out.push({e:e,sc:sc});
    });
    out.sort(function(a,b){return b.sc-a.sc;});
    cur=out.slice(0,30).map(function(o){return o.e;});
    var h='';
    if(!cur.length){ h='<div class="empty">Tidak ada hasil untuk "'+esc(q)+'"</div>'; }
    cur.forEach(function(e,i){
      var where=e.p.indexOf('w')===0? 'Minggu '+parseInt(e.p.slice(1),10)+' · '+tabName(e.t) : pageName(e.p);
      h+='<div class="r'+(i===0?' sel':'')+'" data-i="'+i+'">'
        +'<div class="rp">'+esc(where)+'</div>'
        +'<div class="rt">'+esc(e.ti)+'</div>'
        +'<div class="rx">'+snip(e.x,q)+'</div></div>';
    });
    res.innerHTML=h; res.classList.add('on'); sel=0;
  }
  function tabName(t){ return {materi:'Materi',lab:'Lab',latihan:'Latihan',
    solusi:'Solusi',checklist:'Checklist'}[t]||''; }
  function pageName(p){ var f=DATA.pages.filter(function(x){return x.id===p;})[0]; return f?f.title:''; }
  function snip(txt,q){
    var low=txt.toLowerCase(), i=low.indexOf(q.split(/\s+/)[0]);
    if(i<0) i=0; var a=Math.max(0,i-60), b=Math.min(txt.length,a+220);
    var s=(a>0?'…':'')+txt.slice(a,b)+(b<txt.length?'…':'');
    return esc(s).replace(new RegExp('('+q.split(/\s+/).map(escRe).join('|')+')','ig'),'<mark>$1</mark>');
  }
  function escRe(s){ return s.replace(/[.*+?^${}()|[\]\\]/g,'\\$&'); }
  inp.addEventListener('input',function(){ search(inp.value); });
  inp.addEventListener('keydown',function(e){
    if(e.key==='Escape'){ inp.value=''; res.classList.remove('on'); inp.blur(); }
    if(e.key==='Enter' && cur.length){ pick(sel); }
    if(e.key==='ArrowDown'){ sel=Math.min(sel+1,cur.length-1); mark(); e.preventDefault(); }
    if(e.key==='ArrowUp'){ sel=Math.max(sel-1,0); mark(); e.preventDefault(); }
  });
  res.addEventListener('click',function(e){
    var r=e.target.closest('.r'); if(r) pick(+r.dataset.i);
  });
  function mark(){
    res.querySelectorAll('.r').forEach(function(r,i){ r.classList.toggle('sel', i===sel); });
    var el=res.querySelector('.r.sel'); if(el) el.scrollIntoView({block:'nearest'});
  }
  function pick(i){
    var e=cur[i]; if(!e) return;
    res.classList.remove('on'); inp.value='';
    go(e.p, e.t||'', e.h||'');
  }
  document.addEventListener('click',function(e){
    if(!e.target.closest('#results') && e.target!==inp) res.classList.remove('on');
  });

  /* ---- keyboard ---- */
  document.addEventListener('keydown',function(e){
    if(e.target.tagName==='INPUT'||e.target.tagName==='TEXTAREA') return;
    if(e.key==='/'){ e.preventDefault(); inp.focus(); }
    if(e.key==='t'){ setTheme(st.theme==='dark'?'light':'dark'); }
    if(e.key==='j'||e.key==='k'){
      var i=order.findIndex(function(o){return o.id===st.page;});
      var n=e.key==='j'? order[i+1]:order[i-1];
      if(n) go(n.id,n.tab,'');
    }
  });

  document.getElementById('menubtn').onclick=function(){
    document.getElementById('sidebar').classList.toggle('open');
  };

  /* ---- init ---- */
  refreshProgress();
  var start=st.page||'home';
  var tab=st.tabs[start]||'materi';
  go(start, DATA.weeks.some(function(w){return w.id===start;})?tab:'','');
})();
"""

SHELL = """<!DOCTYPE html>
<html lang="id">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>__TITLE__</title>
<style>
__CSS__
__PYG_LIGHT__
__PYG_DARK__
</style>
</head>
<body>
<div id="app">
  <aside id="sidebar">
    <div class="brand">
      <div>
        <b>Kursus Low-Level C</b>
        <span>12 minggu · fondasi systems engineering</span>
      </div>
    </div>
    <div id="navlist"></div>
  </aside>
  <div id="main">
    <div id="topbar">
      <button class="iconbtn" id="menubtn">Menu</button>
      <input id="search" type="search" placeholder="Cari materi… (tekan / )" autocomplete="off">
      <button class="iconbtn" id="themebtn">Gelap</button>
    </div>
    <div id="results"></div>
    <div id="pages"></div>
  </div>
</div>
<script type="application/json" id="course-data">__DATA__</script>
<script type="application/json" id="search-index">__INDEX__</script>
<script>
__JS__
</script>
</body>
</html>
"""


def json_for_js(obj) -> str:
    return (
        json.dumps(obj, ensure_ascii=False)
        .replace("<", "\\u003c")
        .replace(">", "\\u003e")
        .replace("&", "\\u0026")
    )


def main():
    src = Path(sys.argv[1] if len(sys.argv) > 1 else ".")
    out = Path(sys.argv[2] if len(sys.argv) > 2 else "index.html")

    pages, weeks, index = build(src)

    light = HtmlFormatter(style="default").get_style_defs("html:not(.dark) .code")
    dark = HtmlFormatter(style="monokai").get_style_defs("html.dark .code")

    data = {"pages": pages, "weeks": weeks}

    doc = (
        SHELL.replace("__TITLE__", "Kursus Low-Level C — 12 Minggu")
        .replace("__CSS__", CSS)
        .replace("__PYG_LIGHT__", light)
        .replace("__PYG_DARK__", dark)
        .replace("__DATA__", json_for_js(data))
        .replace("__INDEX__", json_for_js(index))
        .replace("__JS__", JS)
    )
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(doc, encoding="utf-8")
    print(f"OK  {out}  ({out.stat().st_size/1024:.0f} KB)")
    print(f"    halaman statis : {len(pages)}")
    print(f"    minggu         : {len(weeks)}")
    print(f"    indeks carian  : {len(index)} chunk")


if __name__ == "__main__":
    main()
