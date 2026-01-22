import sys
import os
import xml.etree.ElementTree as ET
import json
import webview  # pip install pywebview

# ==========================================
# 1. PARSER & STYLING CONFIG
# ==========================================
def parse_node(element):
    tag = element.tag
    text = element.text.strip() if element.text else ""
    label = tag
    if text: label += f": {text}"

    # GitHub Dark Dimmed Theme Palette
    # Each type has a { fill, stroke }
    style_map = {
        "class":           {"f": "#1f6feb", "s": "#388bfd"}, # Blue
        "subroutineDec":   {"f": "#238636", "s": "#2ea043"}, # Green
        "doStatement":     {"f": "#8957e5", "s": "#a371f7"}, # Purple
        "letStatement":    {"f": "#8957e5", "s": "#a371f7"},
        "ifStatement":     {"f": "#d29922", "s": "#e3b341"}, # Orange
        "whileStatement":  {"f": "#d29922", "s": "#e3b341"},
        "returnStatement": {"f": "#da3633", "s": "#f85149"}, # Red
        "identifier":      {"f": "#30363d", "s": "#6e7681"}, # Dark Gray
        "symbol":          {"f": "#30363d", "s": "#8b949e"},
        "integerConstant": {"f": "#1f6feb", "s": "#58a6ff"},
        "stringConstant":  {"f": "#1f6feb", "s": "#58a6ff"},
        "keyword":         {"f": "#da3633", "s": "#f85149"},
        # Default
        "default":         {"f": "#30363d", "s": "#6e7681"}
    }

    style = style_map.get(tag, style_map["default"])

    return {
        "name": label,
        "fill": style["f"],
        "stroke": style["s"],
        "children": [parse_node(child) for child in element]
    }

# ==========================================
# 2. LOCAL ASSET LOADER
# ==========================================
def get_d3_script():
    """
    Tries to load d3.v7.min.js from the same folder as this script.
    Falls back to CDN if missing.
    """
    script_dir = os.path.dirname(os.path.abspath(__file__))
    d3_path = os.path.join(script_dir, "d3.v7.min.js")

    if os.path.exists(d3_path):
        print(f"[+] Found local D3.js at: {d3_path}")
        try:
            with open(d3_path, "r", encoding="utf-8") as f:
                return f"<script>\n{f.read()}\n</script>"
        except Exception as e:
            print(f"[!] Error reading local D3: {e}")

    print(f"[!] Warning: d3.v7.min.js not found at {d3_path}")
    print("    -> Falling back to online CDN.")
    return '<script src="https://d3js.org/d3.v7.min.js"></script>'

# ==========================================
# 3. APP GENERATOR
# ==========================================
def get_html_content(json_data, d3_script_tag):
    json_str = json.dumps(json_data)

    return f"""
<!DOCTYPE html>
<html>
<head>
    {d3_script_tag}
    <style>
        /* Modern slick UI */
        body {{ 
            margin: 0; 
            background-color: #0d1117; /* GitHub Dimmed BG */
            overflow: hidden; 
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
        }}
        canvas {{ display: block; outline: none; }}
        
        /* Floating HUD */
        #ui-layer {{
            position: fixed;
            top: 20px; left: 20px;
            pointer-events: none;
        }}
        .card {{
            background: rgba(22, 27, 34, 0.95);
            backdrop-filter: blur(10px);
            border: 1px solid #30363d;
            border-radius: 6px;
            padding: 12px 16px;
            color: #c9d1d9;
            box-shadow: 0 4px 12px rgba(0,0,0,0.3);
            margin-bottom: 10px;
        }}
        h1 {{ margin: 0 0 5px 0; font-size: 14px; font-weight: 600; color: #f0f6fc; }}
        p {{ margin: 0; font-size: 12px; color: #8b949e; }}
        .badge {{ display: inline-block; width: 8px; height: 8px; border-radius: 50%; margin-right: 6px; }}
    </style>
</head>
<body>
    <div id="ui-layer">
        <div class="card">
            <h1>Jack Compiler Viz</h1>
            <p>Pan: Left Click &bull; Zoom: Scroll</p>
        </div>
        </div>
    
    <canvas id="viz"></canvas>

    <script>
        const data = {json_str};
        const canvas = document.querySelector("#viz");
        const ctx = canvas.getContext("2d", {{ alpha: false }});
        
        let width = window.innerWidth;
        let height = window.innerHeight;
        let transform = d3.zoomIdentity;

        // --- 1. SETUP D3 LAYOUT ---
        const root = d3.hierarchy(data);
        const treeLayout = d3.tree().nodeSize([40, 200]); // Tighter vertical, wider horizontal
        treeLayout(root);

        // --- 2. HELPER: ROUNDED RECTANGLES ---
        function roundRect(ctx, x, y, width, height, radius) {{
            ctx.beginPath();
            ctx.moveTo(x + radius, y);
            ctx.lineTo(x + width - radius, y);
            ctx.quadraticCurveTo(x + width, y, x + width, y + radius);
            ctx.lineTo(x + width, y + height - radius);
            ctx.quadraticCurveTo(x + width, y + height, x + width - radius, y + height);
            ctx.lineTo(x + radius, y + height);
            ctx.quadraticCurveTo(x, y + height, x, y + height - radius);
            ctx.lineTo(x, y + radius);
            ctx.quadraticCurveTo(x, y, x + radius, y);
            ctx.closePath();
        }}

        // --- 3. RENDER LOOP ---
        function draw() {{
            const k = transform.k;
            const tx = transform.x;
            const ty = transform.y;

            // Clear Background
            ctx.fillStyle = "#0d1117";
            ctx.fillRect(0, 0, width, height);
            
            ctx.save();
            ctx.translate(tx, ty);
            ctx.scale(k, k);

            // A. DRAW LINKS (Smooth Bezier)
            ctx.beginPath();
            ctx.strokeStyle = "#30363d"; // Subtle link color
            ctx.lineWidth = 2;
            
            const linkGen = d3.linkHorizontal()
                .x(d => d.y)
                .y(d => d.x)
                .context(ctx);
            
            root.links().forEach(d => {{
                // Culling
                const sy = d.source.x * k + ty;
                if (sy < -100 || sy > height + 100) return;
                linkGen(d);
            }});
            ctx.stroke();

            // B. DRAW NODES (Rounded Cards)
            ctx.font = "600 12px -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif";
            ctx.textBaseline = "middle";
            ctx.lineWidth = 1.5;

            root.descendants().forEach(d => {{
                // Culling
                const sy = d.x * k + ty;
                const sx = d.y * k + tx;
                if (sy < -50 || sy > height + 50) return;
                if (sx < -200 || sx > width + 50) return;

                const nodeW = Math.max(120, ctx.measureText(d.data.name).width + 30);
                const nodeH = 26;
                const nodeX = d.y;
                const nodeY = d.x - (nodeH / 2);

                // 1. Draw Card Background (Rounded)
                roundRect(ctx, nodeX, nodeY, nodeW, nodeH, 6); // Radius 6px
                ctx.fillStyle = d.data.fill; 
                ctx.fill();

                // 2. Draw Card Border
                ctx.strokeStyle = d.data.stroke;
                ctx.stroke();

                // 3. Draw Text
                if (k > 0.4) {{
                    ctx.fillStyle = "#ffffff"; // White text
                    ctx.fillText(d.data.name, nodeX + 10, d.x + 1);
                }}
            }});

            ctx.restore();
        }}

        // --- 4. INTERACTION ---
        const zoom = d3.zoom()
            .scaleExtent([0.1, 4])
            .on("zoom", e => {{
                transform = e.transform;
                requestAnimationFrame(draw);
            }});

        d3.select(canvas)
            .call(zoom)
            .call(zoom.transform, d3.zoomIdentity.translate(100, height / 2).scale(0.8));

        window.addEventListener("resize", () => {{
            width = window.innerWidth;
            height = window.innerHeight;
            canvas.width = width;
            canvas.height = height;
            draw();
        }});
        
        // High DPI Support
        const dpr = window.devicePixelRatio || 1;
        canvas.width = window.innerWidth * dpr;
        canvas.height = window.innerHeight * dpr;
        ctx.scale(dpr, dpr);

        window.dispatchEvent(new Event('resize'));
    </script>
</body>
</html>
    """

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python jack_viz_slick.py <file.xml>")
        sys.exit(1)

    xml_path = sys.argv[1]
    if os.path.exists(xml_path):
        print("Parsing...")
        tree = ET.parse(xml_path)
        json_data = parse_node(tree.getroot())
        d3_script = get_d3_script()
        html_content = get_html_content(json_data,d3_script)

        print("Launching...")
        webview.create_window(
            title="Jack Visualizer Pro",
            html=html_content,
            width=1280,
            height=800,
            background_color='#0d1117'
        )
        webview.start()