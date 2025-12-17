#!/usr/bin/env python
import ast
import json
import sys

# ===============================
# Execution State
# ===============================

variables = {}
arrays = {}
steps = []
halt_execution = False


# ===============================
# Step Emitters
# ===============================

def emit(step_type, desc, data=None):
    global halt_execution
    if halt_execution:
        return
    steps.append({
        "type": step_type,
        "desc": desc,
        "data": data or {}
    })


def emit_error(message):
    global halt_execution
    emit("error", message)
    halt_execution = True


# ===============================
# Expression Evaluation (LIMITED)
# ===============================

def eval_expr(node):
    if isinstance(node, ast.Constant):
        return node.value

    if isinstance(node, ast.Name):
        if node.id in variables:
            return variables[node.id]
        if node.id in arrays:
            return arrays[node.id]
        emit_error(f"Undefined variable: {node.id}")
        return None

    if isinstance(node, ast.Subscript):
        arr_name = node.value.id
        index = eval_expr(node.slice)
        emit("array_access", f"Access {arr_name}[{index}]",
             {"array": arr_name, "index": index})
        return arrays[arr_name][index]

    if isinstance(node, ast.BinOp):
        left = eval_expr(node.left)
        right = eval_expr(node.right)
        if isinstance(node.op, ast.Add):
            return left + right
        if isinstance(node.op, ast.Sub):
            return left - right
        if isinstance(node.op, ast.Mult):
            return left * right
        if isinstance(node.op, ast.Div):
            return left // right

    emit_error("Unsupported expression")
    return None


# ===============================
# Statement Execution
# ===============================

def exec_assign(node):
    target = node.targets[0]

    # arr = [ ... ]
    if isinstance(target, ast.Name) and isinstance(node.value, ast.List):
        values = [elt.value for elt in node.value.elts]
        arrays[target.id] = values
        emit("array_init",
             f"Initialize array {target.id}",
             {"name": target.id, "values": values})
        return

    # i = value
    if isinstance(target, ast.Name):
        value = eval_expr(node.value)
        variables[target.id] = value
        emit("var_assign",
             f"{target.id} = {value}",
             {"name": target.id, "value": value})
        return

    # arr[i] = value
    if isinstance(target, ast.Subscript):
        arr = target.value.id
        index = eval_expr(target.slice)
        value = eval_expr(node.value)
        arrays[arr][index] = value
        emit("array_update",
             f"{arr}[{index}] = {value}",
             {"array": arr, "index": index, "value": value})
        return

    emit_error("Unsupported assignment")


def exec_for(node):
    if not isinstance(node.iter, ast.Call) or node.iter.func.id != "range":
        emit_error("Only for-loops with range() are supported")
        return

    start = 0
    args = [eval_expr(a) for a in node.iter.args]
    if len(args) == 1:
        end = args[0]
    elif len(args) == 2:
        start, end = args
    else:
        emit_error("Invalid range() usage")
        return

    loop_var = node.target.id
    emit("loop_start", f"Start loop {loop_var}")

    for i in range(start, end):
        variables[loop_var] = i
        emit("var_assign",
             f"{loop_var} = {i}",
             {"name": loop_var, "value": i})

        for stmt in node.body:
            exec_stmt(stmt)
            if halt_execution:
                return

    emit("loop_end", f"End loop {loop_var}")


def exec_if(node):
    left = eval_expr(node.test.left)
    right = eval_expr(node.test.comparators[0])

    emit("compare",
         "Compare values",
         {"left": left, "right": right})

    condition = False
    if isinstance(node.test.ops[0], ast.Gt):
        condition = left > right

    if condition:
        for stmt in node.body:
            exec_stmt(stmt)


def exec_expr(node):
    if isinstance(node.value, ast.Call):
        func = node.value.func.id
        if func == "print":
            val = eval_expr(node.value.args[0])
            emit("print", f"Print {val}", {"value": str(val)})
            return

    emit_error("Unsupported expression statement")


def exec_stmt(node):
    if halt_execution:
        return

    if isinstance(node, ast.Assign):
        exec_assign(node)
    elif isinstance(node, ast.For):
        exec_for(node)
    elif isinstance(node, ast.If):
        exec_if(node)
    elif isinstance(node, ast.Expr):
        exec_expr(node)
    else:
        emit_error(f"Unsupported statement: {type(node).__name__}")


# ===============================
# Main Entry
# ===============================

def main():
    if len(sys.argv) != 2:
        print(json.dumps({"error": "Usage: python_parser_engine.py <file>"}))
        return

    with open(sys.argv[1], "r", encoding="utf-8") as f:
        code = f.read()

    tree = ast.parse(code)

    for stmt in tree.body:
        exec_stmt(stmt)
        if halt_execution:
            break

    output = {
        "language": "python",
        "version": "v1",
        "steps": steps
    }

    print(json.dumps(output, indent=2))


if __name__ == "__main__":
    main()
