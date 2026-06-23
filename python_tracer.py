import sys
import json
import traceback
import io

# ===============================
# Tracer Logic
# ===============================

class ExecutionTracer:
    def __init__(self):
        self.steps = []
        self.call_stack = []

    def serialize_value(self, val, _seen=None):
        """Helper to make data JSON serializable"""
        # Cycle detection for objects with circular references
        if _seen is None:
            _seen = set()

        # 1. Handle Lists, Tuples, AND SETS
        if isinstance(val, (list, tuple, set)):
            return [self.serialize_value(x, _seen) for x in val]
            
        # 2. Handle Dictionaries (Graphs)
        if isinstance(val, dict):
            return {str(k): self.serialize_value(v, _seen) for k, v in val.items()}
            
        # 3. Handle Primitives
        if isinstance(val, (int, float, str, bool, type(None))):
            return val

        # 4. Handle Custom Class Instances (Tree Nodes, Linked Lists, etc.)
        #    Convert their __dict__ into a JSON-friendly dict so the
        #    C++ visualizer can render them as trees/structures.
        if hasattr(val, '__dict__'):
            obj_id = id(val)
            if obj_id in _seen:
                return "<circular ref>"
            _seen.add(obj_id)
            result = {}
            for attr_name, attr_val in val.__dict__.items():
                if attr_name.startswith('_'):
                    continue
                result[attr_name] = self.serialize_value(attr_val, _seen)
            return result
            
        # 5. Fallback for objects
        try:
            return repr(val)
        except Exception:
            return "<Unserializable>"

    def trace_calls(self, frame, event, arg):
        if frame.f_code.co_filename != '<string>':
            return None
            
        if event == 'call':
            func_name = frame.f_code.co_name
            if func_name != '<module>':
                self.call_stack.append(func_name)
            return self.trace_lines
        return None

    def trace_lines(self, frame, event, arg):
        if frame.f_code.co_filename != '<string>':
            return None
        if event not in ['line', 'return']:
            return self.trace_lines

        # 1. Capture Line Number
        line_no = frame.f_lineno

        # 2. Capture Local Variables
        current_locals = {}
        for var_name, var_val in frame.f_locals.items():
            if var_name.startswith("__"): continue 
            if var_name == "self": continue  # Skip 'self' — creates confusing partial trees
            if hasattr(var_val, '__call__') or type(var_val).__name__ == 'module':
                continue
            if isinstance(var_val, type): continue  # Skip class definitions
            
            # Serialize properly (dicts stay dicts, lists stay lists)
            current_locals[var_name] = self.serialize_value(var_val)

        # 2b. Capture referenced globals (e.g., 'visited', 'graph' used in
        #     functions but defined at module scope). Without this, module-level
        #     data structures disappear from the visualization during function calls.
        if frame.f_code.co_name != '<module>':
            for gname in frame.f_code.co_names:
                if gname in current_locals:
                    continue  # local already captured
                if gname not in frame.f_globals:
                    continue
                gval = frame.f_globals[gname]
                if gname.startswith("__"):
                    continue
                if hasattr(gval, '__call__') or type(gval).__name__ == 'module':
                    continue
                if isinstance(gval, type):
                    continue
                current_locals[gname] = self.serialize_value(gval)

        # 3. Create Step
        step = {
            "line": line_no,
            "event": event,
            "func": frame.f_code.co_name,
            "stack": self.call_stack.copy(),
            "variables": current_locals
        }
        self.steps.append(step)
        
        if event == 'return' and self.call_stack:
            self.call_stack.pop()

        return self.trace_lines

    def run(self, code):
        buffer = io.StringIO()
        original_stdout = sys.stdout
        sys.stdout = buffer
        
        try:
            sys.settrace(self.trace_calls)
            exec(code, {'__name__': '__main__'})
        except Exception:
            self.steps.append({
                "event": "error",
                "message": traceback.format_exc()
            })
        finally:
            sys.settrace(None)
            sys.stdout = original_stdout
            
        printed_output = buffer.getvalue()
        if printed_output:
             self.steps.append({
                 "event": "print_output",
                 "message": printed_output
             })

        return json.dumps(self.steps, indent=2)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(json.dumps([{"event": "error", "message": "No input file provided"}]))
        sys.exit(1)

    input_file = sys.argv[1]
    try:
        with open(input_file, 'r') as f:
            user_code = f.read()
        tracer = ExecutionTracer()
        print(tracer.run(user_code))
    except Exception as e:
        print(json.dumps([{"event": "error", "message": str(e)}]))