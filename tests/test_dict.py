a = {}
a["O"] = "G"
assert a != {}, "Dict equals False dict with elements, blank dict"
assert a == {"O": "G"}, "Dict equals with two dicts with filled elements"
a["U"] = {1: 1}
assert a == {"O": "G", "U": {1 :1}}, "Dict equals with a dict inside as a value"
assert a.get("O") == "G", "Dict get without default returns value"
assert a.get("P") == None, "Dict get without default returns none"
assert a.get("P", "default") == "default", "Dict get with default returns default"
assert a.keys() == ["O", "U"], "Dict keys"
assert a.values() == ["G", {1 :1}], "Dict values"
