import sys
import os
import argparse
import atexit
import traceback
from textual.app import App, ComposeResult
from textual.widgets import Header, Footer, TabbedContent, TabPane
from textual.binding import Binding
from global_registry_viz import RegistryBrowser
from symbol_table_viz import SymbolTableBrowser


# ==================================================================================================
# APP: Unified Dashboard
# Combines Registry and Symbol Table views into a tabbed interface.
# ==================================================================================================
class UnifiedDashboard(App):
    CSS = """
    Screen { background: $surface; }
    
    /* SHARED STYLES */
    .search-container { dock: top; height: auto; padding: 0 1; margin-bottom: 1; }
    Input { width: 100%; border: tall $accent; background: $panel; }
    
    /* SPLIT VIEW STYLES (For Symbol Browser) */
    .split-view { height: 1fr; width: 100%; }
    .sidebar { width: 30%; height: 100%; border-right: vkey $accent; background: $panel; }
    .content { width: 70%; height: 100%; padding: 0 1; }
    .header-label { background: $accent; color: white; padding: 0 1; text-align: center; text-style: bold; }
    
    DataTable { height: 1fr; border: none; }
    Tree { height: 1fr; }
    """

    BINDINGS = [
        Binding("q", "quit", "Quit"),
        Binding("1", "switch_tab('tab_registry')", "Registry"),
        Binding("2", "switch_tab('tab_symbols')", "Symbols"),
    ]

    def __init__(self, registry_path, symbol_paths):
        super().__init__()
        self.registry_path = registry_path
        self.symbol_paths = symbol_paths

    def compose(self) -> ComposeResult:
        yield Header(show_clock=True)
        with TabbedContent(initial="tab_registry", id="main_tabs"):
            # TAB 1: Global Registry
            with TabPane("Global Registry", id="tab_registry"):
                yield RegistryBrowser(self.registry_path)

            # TAB 2: Symbol Tables
            with TabPane("Symbol Tables", id="tab_symbols"):
                yield SymbolTableBrowser(self.symbol_paths)
        yield Footer()

    def on_mount(self):
        self.title = "Jack Compiler Dashboard"

    def action_switch_tab(self, tab_id: str):
        """Switches the active tab safely."""
        # Removed broad try/except to allow errors to surface
        tabs = self.query_one("#main_tabs", TabbedContent)
        tabs.active = tab_id

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
    parser = argparse.ArgumentParser(description="Unified Dashboard for Jack Compiler Visualization")
    parser.add_argument("--registry", required=True, help="Path to the global registry JSON file")
    parser.add_argument("--symbols", nargs="+", default=[], help="List of paths to symbol table JSON files")
    
    # Removed try/except around parse_args as argparse handles errors well
    args = parser.parse_args()

    # Register cleanup for all temp files
    files_to_clean = [args.registry] + args.symbols
    atexit.register(cleanup_files, files_to_clean)

    try:
        app = UnifiedDashboard(args.registry, args.symbols)
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
