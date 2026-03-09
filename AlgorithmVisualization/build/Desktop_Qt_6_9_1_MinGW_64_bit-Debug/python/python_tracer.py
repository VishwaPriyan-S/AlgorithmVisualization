import sys
import json
import traceback

# ===============================
# Tracer Logic
# ===============================

class ExecutionTracer:
    def __init__(self):
        self.steps = []
        self.call_stack = []

    def serialize_value(self, val):
        """Helper to make data JSON serializable"""
        if isinstance(val, (list, tuple)):
            return [self.serialize_value(x) for x in val]
        if isinstance(val, (int, float, str, bool, type(None))):
            return val
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

        # Update stack on return
        if event == 'return' and self.call_stack:
             # We capture the state BEFORE popping for visualization purposes
             # or we can handle it in the next step. 
             pass 

        # 1. Capture Line Number
        line_no = frame.f_lineno

        # 2. Capture Local Variables
        current_locals = {}
        for var_name, var_val in frame.f_locals.items():
            if var_name.startswith("__"): continue 
            # Skip modules and functions to keep output clean
            if hasattr(var_val, '__call__') or type(var_val).__name__ == 'module':
                continue
            
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
        try:
            sys.settrace(self.trace_calls)
            exec(code, {'__name__': '__main__'})
        except Exception:
            # Capture runtime errors
            self.steps.append({
                "event": "error",
                "message": traceback.format_exc()
            })
        finally:
            sys.settrace(None)
            
        return json.dumps(self.steps, indent=2)

# ===============================
# Main Entry Point
# ===============================

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(json.dumps([{"event": "error", "message": "No input file provided"}]))
        sys.exit(1)

    input_file = sys.argv[1]
    
    with open(input_file, 'r') as f:
        user_code = f.read()

    tracer = ExecutionTracer()
    print(tracer.run(user_code))