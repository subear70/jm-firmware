/* Shared renderer for the Jammer HTML docs.
 * Reads the embedded Markdown from <script id="md" type="text/markdown">,
 * renders it with marked, renders ```mermaid blocks with mermaid, builds the
 * sidebar nav, and rewrites .md links to .html. Works offline from file://.
 */
(function () {
  var PAGES = [
    { file: "index.html", title: "Overview" },
    { file: "modbus-protocol-spec.html", title: "Modbus Protocol" },
    { file: "desktop-modbus-controller-spec.html", title: "Desktop Controller" },
    { file: "stm32-waveform-node-spec.html", title: "STM32 Firmware" },
  ];

  function escapeHtml(s) {
    return s
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;");
  }

  function currentFile() {
    var parts = location.pathname.split("/");
    var f = parts[parts.length - 1];
    return f === "" ? "index.html" : f;
  }

  function buildSidebar() {
    var here = currentFile();
    var nav = document.createElement("nav");
    nav.className = "sidebar";
    var html = '<a class="brand" href="index.html">Jammer&nbsp;Docs</a><ul>';
    PAGES.forEach(function (p) {
      var active = p.file === here ? ' class="active"' : "";
      html += "<li><a" + active + ' href="' + p.file + '">' + p.title + "</a></li>";
    });
    html += "</ul>";
    nav.innerHTML = html;
    document.body.insertBefore(nav, document.body.firstChild);
  }

  function buildPageNav(main) {
    var here = currentFile();
    var idx = PAGES.findIndex(function (p) { return p.file === here; });
    if (idx < 0) return;
    var prev = PAGES[idx - 1];
    var next = PAGES[idx + 1];
    var nav = document.createElement("div");
    nav.className = "page-nav";
    nav.innerHTML =
      (prev
        ? '<a class="prev" href="' + prev.file + '"><span>Previous</span>' + prev.title + "</a>"
        : "<span></span>") +
      (next
        ? '<a class="next" href="' + next.file + '"><span>Next</span>' + next.title + "</a>"
        : "<span></span>");
    main.appendChild(nav);
  }

  function render() {
    var src = document.getElementById("md").textContent;

    // Extract mermaid fenced blocks so marked leaves them untouched.
    var blocks = [];
    var cleaned = src.replace(/```mermaid\r?\n([\s\S]*?)```/g, function (_, code) {
      blocks.push(code);
      return "\n@@MERMAID" + (blocks.length - 1) + "@@\n";
    });

    marked.setOptions({ gfm: true, breaks: false, headerIds: true, mangle: false });
    var html = marked.parse(cleaned);

    // Swap placeholders back to <pre class="mermaid"> (labels: \n -> <br/>).
    html = html.replace(/(?:<p>)?@@MERMAID(\d+)@@(?:<\/p>)?/g, function (_, i) {
      var code = blocks[i].replace(/\\n/g, "<br/>");
      return '<pre class="mermaid">' + escapeHtml(code) + "</pre>";
    });

    var main = document.createElement("main");
    main.innerHTML = html;
    document.body.appendChild(main);

    // Rewrite .md links to .html (handles plain and #anchor forms).
    main.querySelectorAll('a[href]').forEach(function (a) {
      var href = a.getAttribute("href");
      if (/\.md(#|$)/.test(href)) {
        a.setAttribute("href", href.replace(/\.md(#|$)/, ".html$1"));
      }
    });

    buildSidebar();
    buildPageNav(main);

    var dark = window.matchMedia && window.matchMedia("(prefers-color-scheme: dark)").matches;
    mermaid.initialize({
      startOnLoad: false,
      securityLevel: "loose",
      theme: dark ? "dark" : "default",
    });
    mermaid.run({ querySelector: "pre.mermaid" });

    document.title = (main.querySelector("h1") ? main.querySelector("h1").textContent + " — " : "") + "Jammer Docs";
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", render);
  } else {
    render();
  }
})();
