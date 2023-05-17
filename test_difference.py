import json

with open("all_words.json", "r") as f:
    new_list = json.load(f)

with open("all_words_referance.json", "r") as f:
    ref_list = json.load(f)

print(f"original list {len(ref_list)}")
print(f"new list {len(new_list)}")

new_map = {}
for word_info in new_list:
    new_map[word_info[0]] = word_info[1]


reduced_by_words = len(ref_list)-len(new_list)
reduced_procent = (len(ref_list)/reduced_by_words)

for word_info in ref_list:
    for word in word_info[0].split("-"):
        if word not in new_map:
            print(word_info[0])

print(f"reduced original list by {reduced_by_words} words ({reduced_procent})%")
