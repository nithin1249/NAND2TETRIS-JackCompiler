import json
import os
import sys
import atexit
import traceback
from textual.app import App, ComposeResult
from textual.widgets import Header, Footer, DataTable, Tree, Label
from textual.containers import Container, Vertical, Horizontal
from textual.binding import Binding

# ==================================================================================================
# WIDGET: Symbol Table Browser
# A split-view widget: File Tree (left) + Symbol Table (right).
# ==================================================================================================
class SymbolTableBrowser(Container):
    """
    Displays a hierarchical view of symbol tables.
    Left: Tree of classes and their subroutines.
    Right: Table of symbols for the selected scope.
    """

    def __init__(self, symbol_paths, **kwargs):
        super().__init__(**kwargs)
        self.symbol_paths = symbol_paths
        self.data_cache = {} # Cache loaded JSON data by class name

    def compose(self) -> ComposeResult:
        with Horizontal(classes="split-view"):
            # LEFT SIDEBAR: Navigation Tree
            with Vertical(classes="sidebar"):
                yield Label("📂 Project Files", classes="header-label")
                yield Tree("Files", id="file_tree")

            # RIGHT CONTENT: Symbol Table
            with Vertical(classes="content"):
                yield Label("Select a scope to view symbols", id="sym_title", classes="header-label")
                yield DataTable(id="sym_table", zebra_stripes=True)

    def on_mount(self):
        """Initialize the widget after mounting."""
        # Removed broad try/except to allow errors to surface
        self.load_files()

    def load_files(self):
        """Loads JSON files and populates the navigation tree."""
        # Removed broad try/except to allow errors to surface
        tree = self.query_one("#file_tree", Tree)
        tree.root.expand()

        table = self.query_one("#sym_table", DataTable)
        table.add_columns("Name", "Type", "Kind", "Index")

        for path in self.symbol_paths:
            if not os.path.exists(path):
                self.notify(f"File not found: {path}", severity="warning")
                continue
            
            try:
                with open(path, 'r') as f:
                    data = json.load(f)

                # Use class name as key, fallback to filename
                name = data.get("className", os.path.basename(path))
                self.data_cache[name] = data

                # Add File Node (Class)
                file_node = tree.root.add(f"📄 {name}", expand=True)
                file_node.data = {"type": "file", "class": name}

                # Add Scopes
                # 1. Class Scope (Static/Field)
                c_node = file_node.add("🔒 Class Scope")
                c_node.data = {"type": "scope", "class": name, "scope": "class"}

                # 2. Subroutines (Local/Argument)
                subroutines = data.get("subroutines", [])
                if isinstance(subroutines, list):
                    for sub in subroutines:
                        sub_name = sub.get('name', 'unknown')
                        s_node = file_node.add(f"ƒ {sub_name}")
                        s_node.data = {"type": "scope", "class": name, "scope": sub_name}
                else:
                    self.notify(f"Invalid 'subroutines' format in {name}", severity="warning")
                    # We don't raise here to allow partial loading of other files, 
                    # but we notify. If strictness is required, we could raise.

            except json.JSONDecodeError as e:
                self.notify(f"Invalid JSON in {path}: {e}", severity="error")
                raise # Re-raise to ensure visibility
            except Exception as e:
                self.notify(f"Error loading {path}: {e}", severity="error")
                raise # Re-raise to ensure visibility

    def on_tree_node_selected(self, event: Tree.NodeSelected):
        """Handles tree node selection to update the symbol table."""
        # Removed broad try/except to allow errors to surface
        if not event.node.data: return
        meta = event.node.data

        if meta.get("type") == "scope":
            self.show_symbol_table(meta.get("class"), meta.get("scope"))

    def show_symbol_table(self, class_name, scope_name):
        """Populates the data table with symbols for the selected scope."""
        # Removed broad try/except to allow errors to surface
        table = self.query_one("#sym_table", DataTable)
        label = self.query_one("#sym_title", Label)

        table.clear()
        data = self.data_cache.get(class_name, {})

        symbols = []
        if scope_name == "class":
            label.update(f"Scope: {class_name} (Static/Fields)")
            symbols = data.get("classSymbols", [])
        else:
            label.update(f"Scope: {class_name}.{scope_name}")
            subroutines = data.get("subroutines", [])
            if isinstance(subroutines, list):
                for sub in subroutines:
                    if sub.get('name') == scope_name:
                        symbols = sub.get('symbols', [])
                        break

        if not isinstance(symbols, list):
            self.notify(f"Invalid symbols format for {scope_name}", severity="error")
            raise ValueError(f"Invalid symbols format for {scope_name}")

        # Sort: Kind -> Index
        # Removed try/except around sort
        symbols.sort(key=lambda x: (x.get('kind', ''), x.get('index', 0)))

        for s in symbols:
            # Removed per-row try/except
            kind = s.get('kind', 'unknown')
            name = s.get('name', '?')
            type_ = s.get('type', '?')
            index = s.get('index', '?')

            kind_icon = "⚪"
            if kind == "local": kind_icon = "📦"
            elif kind == "argument": kind_icon = "📥"
            elif kind == "static": kind_icon = "💾"
            elif kind == "field": kind_icon = "🏷️"

            table.add_row(name, type_, f"{kind_icon} {kind}", str(index))

# ==================================================================================================
# APP: Symbol Table App
# Main application wrapper.
# ==================================================================================================
class SymbolTableApp(App):
    CSS = """
    Screen { background: $surface; }
    .split-view { height: 1fr; width: 100%; }
    .sidebar { width: 30%; height: 100%; border-right: vkey $accent; background: $panel; }
    .content { width: 70%; height: 100%; padding: 0 1; }
    .header-label { background: $accent; color: white; padding: 0 1; text-align: center; text-style: bold; }
    DataTable { height: 1fr; border: none; }
    Tree { height: 1fr; }
    """
    BINDINGS = [Binding("q", "quit", "Quit")]

    def __init__(self, files):
        super().__init__()
        self.files = files

    def compose(self) -> ComposeResult:
        yield SymbolTableBrowser(self.files)
        yield Footer()

    def on_mount(self):
        self.title = "Symbol Table Viewer"

# ==================================================================================================
# UTILS & MAIN
# ==================================================================================================
def cleanup_files(paths):
    """Attempts to remove temporary files on exit."""
    for p in paths:
        if p and os.path.exists(p):
            try: 
                os.remove(p)
            except OSError as e:
                print(f"Warning: Could not remove temp file {p}: {e}", file=sys.stderr)
            # Removed generic Exception pass

if __name__ == "__main__":
    if len(sys.argv) < 2:
        # Silent exit if no files provided (likely invoked incorrectly)
        sys.exit(1)

    files = sys.argv[1:]
    
    # Register cleanup
    atexit.register(cleanup_files, files)

    try:
        app = SymbolTableApp(files)
        app.run()
    except KeyboardInterrupt:
        print("\nInterrupted by user")
        sys.exit(0)
    except Exception as e:
        # Ensure terminal is reset even on crash
        os.system('cls' if os.name == 'nt' else 'clear')
        print(f"Fatal Error: {e}", file=sys.stderr)
        traceback.print_exc()
        sys.exit(1)
