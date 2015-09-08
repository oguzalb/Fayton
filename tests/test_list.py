l = []
l.append(1)
assert l == [1], "append"
l.extend([2, 3])
assert l == [1, 2, 3], "extend"
assert l[0:-1] == [1, 2], "slice till last"
assert l[:] == [1, 2, 3], "copy"
assert l[::-1] == [3, 2, 1], "reverse slice"
l.reverse()
assert l == [3, 2, 1], "reverse"
assert l.pop() == 1, "pop"
l[0] = 1
l[1] = 2
assert l == [1, 2], "set_item"
assert l[0] == 1, "get_item first"
assert l[1] == 2, "get_item last"
