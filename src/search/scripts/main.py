import json
from unidecode import unidecode

f = open("dict.json", "r")
val = json.loads(f.read())
new_json = json.loads("{}")

for x in val:
    new_key = unidecode(x)
    new_json[new_key] = val[x]
    new_json[new_key]["bf"] = unidecode(val[x]["bf"])


save = open("new_dict.json","w")
save.write(json.dumps(new_json))


