import json
import os
import sys
import atexit
import traceback
from textual.app import App, ComposeResult
from textual.widgets import Header, Footer, DataTable, Input
from textual.containers import Container
from textual.binding import Binding

# ==================================================================================================
# WIDGET: Registry Browser
# Displays the registry data in a searchable table.
# ==================================================================================================
class RegistryBrowser(Container):
    def __init__(self, json_path, **kwargs):
        super().__init__(**kwargs)
        self.json_path = json_path
        self.raw_data = [] # Stores the full dataset for filtering

    def compose(self) -> ComposeResult:
        # 1. Search Bar
        yield Container(
            Input(placeholder=f"Search Registry...", id="reg_search"),
            classes="search-container"
        )
        # 2. Data Table
        yield DataTable(id="reg_table", zebra_stripes=True)

    def on_mount(self):
        """Initialize the table columns and load data on startup."""
        # Removed broad try/except to allow errors to surface
        table = self.query_one("#reg_table", DataTable)
        table.cursor_type = "row"
        table.add_columns("Class", "Method", "Type", "Return", "Parameters")
        self.load_data()

    def load_data(self):
        """Reads the JSON file and populates the table."""
        # Removed broad try/except to allow errors to surface
        table = self.query_one("#reg_table", DataTable)
        table.clear()

        # 1. File Existence Check
        if not os.path.exists(self.json_path):
            self.notify(f"JSON file not found: {self.json_path}", severity="warning")
            return

        # 2. Parse JSON
        # We keep specific exceptions for IO/JSON as they are expected runtime conditions
        try:
            with open(self.json_path, 'r') as f:
                data = json.load(f)
        except json.JSONDecodeError as e:
            self.notify(f"Invalid JSON format: {e}", severity="error")
            raise # Re-raise to ensure visibility in logs/traceback if needed
        except IOError as e:
            self.notify(f"IO Error reading file: {e}", severity="error")
            raise

        # 3. Validate Structure
        registry = data.get("registry", [])
        if not isinstance(registry, list):
            self.notify("Invalid JSON structure: 'registry' key must be a list", severity="error")
            raise ValueError("Invalid JSON structure: 'registry' key must be a list")

        # 4. Sort Data (Class -> Method)
        # Removed try/except around sort to expose key errors or type errors
        registry.sort(key=lambda x: (x.get('class', ''), x.get('method', '')))

        # 5. Populate Table
        for item in registry:
            # Removed per-row try/except to expose malformed data
            
            # Safe extraction with defaults is still good practice, 
            # but if something is critically wrong (e.g. item is not a dict), we let it crash.
            class_name = item.get('class', '<unknown>')
            method_name = item.get('method', '<unknown>')
            item_type = item.get('type', 'unknown')
            return_type = item.get('return', 'void')
            params = item.get('params', [])
            
            if not isinstance(params, list):
                params = str(params) # Fallback if params is not a list

            # Format the 'Type' column with icons
            kind = "ƒ static" if item_type == 'function' else "ⓜ method"
            if item_type == 'constructor': kind = "🔨 new"

            row = (class_name, method_name, kind, return_type, str(params))
            self.raw_data.append(row)
            table.add_row(*row)

    def on_input_changed(self, event: Input.Changed):
        """Filters the table based on the search input."""
        # Removed broad try/except to allow errors to surface
        if event.input.id == "reg_search":
            query = event.value.lower()
            table = self.query_one("#reg_table", DataTable)
            table.clear()
            
            # Re-populate table with matching rows
            for row in self.raw_data:
                # Safe search across all columns
                if any(query in str(cell).lower() for cell in row):
                    table.add_row(*row)

# ==================================================================================================
# APP: Registry Dashboard
# Main application class.
# ==================================================================================================
class RegistryDashboard(App):
    CSS = """
    Screen { layout: vertical; background: $surface; }
    .search-container { dock: top; height: auto; padding: 0 1; margin-bottom: 1; }
    Input { width: 100%; border: tall $accent; background: $panel; }
    DataTable { height: 1fr; border: none; }
    DataTable > .datatable--header { text-style: bold; background: $accent; color: auto; }
    """

    BINDINGS = [Binding("q", "quit", "Quit")]

    def __init__(self, json_path):
        super().__init__()
        self.json_path = json_path

    def compose(self) -> ComposeResult:
        yield RegistryBrowser(self.json_path)
        yield Footer()

    def on_mount(self):
        self.title = "Global Registry"

# ==================================================================================================
# UTILS & MAIN
# ==================================================================================================
def cleanup_temp_file(path):
    """Attempts to remove the temporary JSON file on exit."""
    if path and os.path.exists(path):
        try: 
            os.remove(path)
        except OSError as e:
            print(f"Warning: Could not remove temp file {path}: {e}", file=sys.stderr)
        # Removed generic Exception pass to allow other errors to surface if any

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 global_registry_viz.py <path_to_json>")
        sys.exit(1)

    json_path = sys.argv[1]
    
    # Register cleanup handler
    atexit.register(cleanup_temp_file, json_path)

    try:
        app = RegistryDashboard(json_path)
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
