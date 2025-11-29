#!/usr/bin/env python
import ast
import json
import sys
from typing import Any

def node_to_dict(node: Any):
    """Recursively convert an AST node into a JSON-serializable dict."""
    if isinstance(node, ast.AST):
        result = {"_type": node.__class__.__name__}
        for field, value in ast.iter_fields(node):
            result[field] = node_to_dict(value)
        return result
    elif isinstance(node, list):
        return [node_to_dict(item) for item in node]
    else:
        # basic types: str, int, None, etc.
        return node

def main():
    if len(sys.argv) != 2:
        print(json.dumps({"error": "Usage", "message": "python_parser_engine.py <source_file>"}))
        sys.exit(1)

    source_path = sys.argv[1]

    try:
        with open(source_path, "r", encoding="utf-8") as f:
            code = f.read()
    except Exception as e:
        print(json.dumps({"error": "FileReadError", "message": str(e)}))
        sys.exit(1)

    try:
        tree = ast.parse(code, mode="exec")
    except SyntaxError as e:
        print(json.dumps({
            "error": "SyntaxError",
            "message": str(e),
            "lineno": e.lineno,
            "offset": e.offset,
            "text": e.text
        }))
        sys.exit(1)
    except Exception as e:
        print(json.dumps({"error": "ParseError", "message": str(e)}))
        sys.exit(1)

    ast_dict = node_to_dict(tree)
    print(json.dumps(ast_dict, indent=2))

if __name__ == "__main__":
    main()
