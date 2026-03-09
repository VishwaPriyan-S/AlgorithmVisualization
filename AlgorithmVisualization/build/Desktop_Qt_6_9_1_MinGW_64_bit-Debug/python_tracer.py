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

    def serialize_value(self, val):
        """Helper to make data JSON serializable"""
        # 1. Handle Lists, Tuples, AND SETS (Sets were missing!)
        if isinstance(val, (list, tuple, set)):
            # Convert to list for JSON compatibility
            return [self.serialize_value(x) for x in val]
            
        # 2. Handle Dictionaries (Graphs) - THIS WAS THE MISSING PART
        if isinstance(val, dict):
            # JSON keys must be strings, so we convert keys like 1 -> "1"
            return {str(k): self.serialize_value(v) for k, v in val.items()}
            
        # 3. Handle Primitives
        if isinstance(val, (int, float, str, bool, type(None))):
            return val
            
        # 4. Fallback for objects
        return str(val)

    def trace_calls(self, frame, event, arg):
        if event == 'call':
            func_name = frame.f_code.co_name
            if func_name != '<module>':
                self.call_stack.append(func_name)
            return self.trace_lines
        return None

    def trace_lines(self, frame, event, arg):
        if event not in ['line', 'return']:
            return self.trace_lines

        # 1. Capture Line Number
        line_no = frame.f_lineno

        # 2. Capture Local Variables
        current_locals = {}
        for var_name, var_val in frame.f_locals.items():
            if var_name.startswith("__"): continue 
            if hasattr(var_val, '__call__') or type(var_val).__name__ == 'module':
                continue
            
            # Serialize properly (dicts stay dicts, lists stay lists)
            current_locals[var_name] = self.serialize_value(var_val)

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