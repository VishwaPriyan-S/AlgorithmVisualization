#!/usr/bin/env python
import ast
import json
import sys

# ===============================
# Execution State
# ===============================

variables = {}
arrays = {}
functions = {}
steps = []
call_stack = []
halt_execution = False

BREAK = "BREAK"
CONTINUE = "CONTINUE"
RETURN = "RETURN"
NONE = None

MAX_CALL_DEPTH = 50

# ===============================
# Step Emitters
# ===============================

def emit(t, desc, data=None, node=None):
    if not halt_execution:
        step = {
            "type": t,
            "desc": desc,
            "data": data or {}
        }
        # Capture line number for highlighting
        if node and hasattr(node, 'lineno'):
            step["line"] = node.lineno
        steps.append(step)

def emit_error(msg, node=None):
    global halt_execution
    emit("error", msg, node=node)
    halt_execution = True

# ===============================
# Expression Evaluation
# ===============================

def eval_expr(node):
    # -------- literals --------
    if isinstance(node, ast.Constant):
        return node.value

    # -------- variable / array --------
    if isinstance(node, ast.Name):
        if node.id in variables:
            return variables[node.id]
        if node.id in arrays:
            return arrays[node.id]
        emit_error(f"Undefined variable: {node.id}", node)
        return None

    # -------- unary --------
    if isinstance(node, ast.UnaryOp):
        if isinstance(node.op, ast.Not):
            return not eval_expr(node.operand)
        if isinstance(node.op, ast.USub):
            return -eval_expr(node.operand)

    # -------- boolean --------
    if isinstance(node, ast.BoolOp):
        if isinstance(node.op, ast.And):
            return all(eval_expr(v) for v in node.values)
        if isinstance(node.op, ast.Or):
            return any(eval_expr(v) for v in node.values)

    # -------- comparison --------
    if isinstance(node, ast.Compare):
        left = eval_expr(node.left)
        right = eval_expr(node.comparators[0])

        op = node.ops[0]
        if isinstance(op, ast.Lt): return left < right
        if isinstance(op, ast.LtE): return left <= right
        if isinstance(op, ast.Gt): return left > right
        if isinstance(op, ast.GtE): return left >= right
        if isinstance(op, ast.Eq): return left == right
        if isinstance(op, ast.NotEq): return left != right

        emit_error("Unsupported comparison", node)
        return None

    # -------- len() --------
    if isinstance(node, ast.Call) and isinstance(node.func, ast.Name):
        if node.func.id == "len":
            return len(eval_expr(node.args[0]))

    # -------- function call --------
    if isinstance(node, ast.Call) and isinstance(node.func, ast.Name):
        fname = node.func.id

        if fname not in functions:
            emit_error(f"Undefined function: {fname}", node)
            return None

        if len(call_stack) >= MAX_CALL_DEPTH:
            emit_error("Maximum recursion depth exceeded", node)
            return None

        func = functions[fname]
        args = [eval_expr(a) for a in node.args]

        emit("func_call", f"Call {fname}", {"name": fname, "args": args}, node)

        # save scope
        call_stack.append(variables.copy())
        variables.clear()

        # bind params
        for p, v in zip(func.args.args, args):
            variables[p.arg] = v

        ret = None
        for stmt in func.body:
            r = exec_stmt(stmt)
            if isinstance(r, tuple) and r[0] == RETURN:
                ret = r[1]
                break

        emit("func_end", f"End {fname}", {"name": fname, "return": ret}, node)

        # restore scope
        prev = call_stack.pop()
        variables.clear()
        variables.update(prev)

        return ret

    # -------- indexing --------
    if isinstance(node, ast.Subscript):
        obj = eval_expr(node.value)
        idx = eval_expr(node.slice)
        
        arr_name = node.value.id if isinstance(node.value, ast.Name) else "?"
        emit("array_access", f"Access {arr_name}[{idx}]", {"array": arr_name, "index": idx}, node)
        return obj[idx]

    # -------- arithmetic --------
    if isinstance(node, ast.BinOp):
        l = eval_expr(node.left)
        r = eval_expr(node.right)
        if isinstance(node.op, ast.Add): return l + r
        if isinstance(node.op, ast.Sub): return l - r
        if isinstance(node.op, ast.Mult): return l * r
        if isinstance(node.op, ast.Div): return l // r

    emit_error("Unsupported expression", node)
    return None

# ===============================
# Condition Evaluation
# ===============================

def eval_condition(test):
    return bool(eval_expr(test))

# ===============================
# Statement Execution
# ===============================

def exec_assign(node):
    t = node.targets[0]

    # 1. Array Initialization
    if isinstance(t, ast.Name) and isinstance(node.value, ast.List):
        vals = [eval_expr(e) for e in node.value.elts]
        arrays[t.id] = vals
        emit("array_init", f"Init array {t.id}", {"name": t.id, "values": vals}, node)
        return NONE

    # 2. Variable Assignment
    if isinstance(t, ast.Name):
        v = eval_expr(node.value)
        variables[t.id] = v
        emit("var_assign", f"{t.id} = {v}", {"name": t.id, "value": v}, node)
        return NONE
        
    # 3. Single Array Item Assignment
    if isinstance(t, ast.Subscript) and isinstance(t.value, ast.Name):
        arr_name = t.value.id
        idx = eval_expr(t.slice)
        val = eval_expr(node.value)
        
        if arr_name in arrays:
            arrays[arr_name][idx] = val
            emit("array_update", f"{arr_name}[{idx}] = {val}", 
                 {"array": arr_name, "index": idx, "value": val}, node)
            return NONE

    # 4. Tuple Unpacking (Handles Swap: a,b = b,a)
    if isinstance(t, ast.Tuple):
        if isinstance(node.value, ast.Tuple):
            values = [eval_expr(e) for e in node.value.elts]
        else:
            emit_error("Complex unpacking not supported", node)
            return NONE
            
        if len(t.elts) == len(values):
            for i, target in enumerate(t.elts):
                val = values[i]
                if isinstance(target, ast.Name):
                    variables[target.id] = val
                    emit("var_assign", f"{target.id} = {val}", 
                         {"name": target.id, "value": val}, node)
                elif isinstance(target, ast.Subscript) and isinstance(target.value, ast.Name):
                    arr_name = target.value.id
                    idx = eval_expr(target.slice)
                    if arr_name in arrays:
                        arrays[arr_name][idx] = val
                        emit("array_update", f"{arr_name}[{idx}] = {val}", 
                             {"array": arr_name, "index": idx, "value": val}, node)
            return NONE

    emit_error("Unsupported assignment", node)
    return NONE

def exec_aug_assign(node):
    # Handles: x += 1, arr[i] -= 5
    target = node.target
    val = eval_expr(node.value)
    current = None
    
    # 1. Get Current Value
    if isinstance(target, ast.Name):
        if target.id in variables:
            current = variables[target.id]
        else:
            emit_error(f"Undefined variable: {target.id}", node)
            return NONE
    elif isinstance(target, ast.Subscript):
        arr_name = target.value.id
        idx = eval_expr(target.slice)
        if arr_name in arrays:
            current = arrays[arr_name][idx]
        else:
            emit_error(f"Undefined array: {arr_name}", node)
            return NONE
    
    # 2. Compute New Value
    res = 0
    if isinstance(node.op, ast.Add): res = current + val
    elif isinstance(node.op, ast.Sub): res = current - val
    elif isinstance(node.op, ast.Mult): res = current * val
    elif isinstance(node.op, ast.Div): res = current // val # Integer div for visualization
    else:
        emit_error("Unsupported augmented operator", node)
        return NONE
        
    # 3. Store Result
    if isinstance(target, ast.Name):
        variables[target.id] = res
        emit("var_assign", f"{target.id} = {res}", {"name": target.id, "value": res}, node)
    elif isinstance(target, ast.Subscript):
        arr_name = target.value.id
        idx = eval_expr(target.slice)
        arrays[arr_name][idx] = res
        emit("array_update", f"{arr_name}[{idx}] = {res}", 
             {"array": arr_name, "index": idx, "value": res}, node)
             
    return NONE

def exec_for(node):
    iterable = eval_expr(node.iter) if not isinstance(node.iter, ast.Call) \
        else range(*[eval_expr(a) for a in node.iter.args])

    emit("loop_start", "For loop start", {}, node)
    broke = False

    for v in iterable:
        variables[node.target.id] = v
        emit("var_assign", f"{node.target.id} = {v}", {"name": node.target.id, "value": v}, node)
        emit("loop_iter_start", f"Iteration {v}", {}, node)

        for s in node.body:
            r = exec_stmt(s)
            if r == BREAK:
                broke = True
                emit("loop_end", "Break loop", {}, s)
                return NONE
            if r == CONTINUE:
                break
            if isinstance(r, tuple) and r[0] == RETURN:
                return r

    if not broke:
        for s in node.orelse:
            exec_stmt(s)

    emit("loop_end", "For loop end", {}, node)
    return NONE

def exec_while(node):
    emit("loop_start", "While loop start", {}, node)
    broke = False
    guard = 0

    while eval_condition(node.test):
        emit("loop_iter_start", "Iteration", {}, node)

        for s in node.body:
            r = exec_stmt(s)
            if r == BREAK:
                broke = True
                emit("loop_end", "Break loop", {}, s)
                return NONE
            if r == CONTINUE:
                break
            if isinstance(r, tuple) and r[0] == RETURN:
                return r

        guard += 1
        if guard > 10000:
            emit_error("Infinite loop detected", node)
            return NONE

    if not broke:
        for s in node.orelse:
            exec_stmt(s)

    emit("loop_end", "While loop end", {}, node)
    return NONE

def exec_if(node):
    condition = eval_condition(node.test)
    block = node.body if condition else node.orelse
    for s in block:
        r = exec_stmt(s)
        if r in (BREAK, CONTINUE) or isinstance(r, tuple):
            return r
    return NONE

def exec_expr(node):
    if isinstance(node.value, ast.Call) and isinstance(node.value.func, ast.Name):
        if node.value.func.id == "print":
            vals = [eval_expr(a) for a in node.value.args]
            emit("print", "Print", {"value": " ".join(map(str, vals))}, node)
            return NONE
    eval_expr(node.value)
    return NONE

def exec_stmt(node):
    if halt_execution:
        return NONE

    if isinstance(node, ast.Assign): return exec_assign(node)
    if isinstance(node, ast.AugAssign): return exec_aug_assign(node) # New Handler
    if isinstance(node, ast.For): return exec_for(node)
    if isinstance(node, ast.While): return exec_while(node)
    if isinstance(node, ast.If): return exec_if(node)

    if isinstance(node, ast.FunctionDef):
        functions[node.name] = node
        emit("func_def", f"Define {node.name}", {}, node)
        return NONE

    if isinstance(node, ast.Return):
        val = eval_expr(node.value) if node.value else None
        emit("return", "Return", {"value": val}, node)
        return (RETURN, val)

    if isinstance(node, ast.Break):
        emit("break", "Break", {}, node)
        return BREAK

    if isinstance(node, ast.Continue):
        emit("continue", "Continue", {}, node)
        return CONTINUE

    if isinstance(node, ast.Expr):
        return exec_expr(node)

    emit_error(f"Unsupported statement: {type(node).__name__}", node)
    return NONE

# ===============================
# Main
# ===============================

def main():
    try:
        with open(sys.argv[1], "r", encoding="utf-8") as f:
            code = f.read()
            tree = ast.parse(code)

        for stmt in tree.body:
            exec_stmt(stmt)
            if halt_execution:
                break
                
        print(json.dumps({
            "language": "python",
            "status": "success",
            "steps": steps
        }, indent=2))
        
    except Exception as e:
        print(json.dumps({
            "language": "python",
            "status": "error",
            "message": str(e),
            "steps": steps
        }, indent=2))

if __name__ == "__main__":
    main()