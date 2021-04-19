# convert_wiki.py

import json
import copy
import os
import random

def main():
    # Load json template.
    with open('metadata_template.json') as json_file:
        template = json.load(json_file) 

    print(f"Got template: {template}")

    # Load wiki-articles
    unprocessed_wiki_jsons = []
    cnt = 1
    with open('wiki-articles-1000.json') as wikis:
        line = wikis.readline()
        while line:
            unprocessed_wiki_jsons.append(json.loads(line))
            line = wikis.readline()
            cnt += 1

    print(f"Read {cnt} lines. First json:\n {unprocessed_wiki_jsons[0]}\n")

    # Process wiki-articles (add new json entry and create new directory for each)
    ran_keys = 10000
    proccessed_wikis = []
    for wiki_json in unprocessed_wiki_jsons:
        new_entry = copy.deepcopy(template)
        new_entry["data"]["date"] = "1999"

        ran_keys += 1
        new_entry["key"] = str(ran_keys)
        new_entry["data"]["key"] = new_entry["key"]

        new_entry["data"]["creators"][0]["firstName"] = "ayyyy"
        new_entry["data"]["creators"][0]["lastName"] = "axxxx"
        new_entry["data"]["creators"][1]["firstName"] = "eyyyy"
        new_entry["data"]["creators"][1]["lastName"] = "exxxx"

        new_entry["data"]["title"] = wiki_json["title"]
        new_entry["data"]["place"] = "pppp"

        new_entry["bib"] = wiki_json["title"]
        new_entry["rights"] = "CLASfrei"
        proccessed_wikis.append(new_entry)

        # Create book directory and ocr
        path = "../../tests/example_data/wiki_data/test_books/" + str(new_entry["key"])
        os.mkdir(path)
        f = open(path+"/ocr.txt", "a")
        f.write(wiki_json["body"])
        f.close()

    print(f"Created entries 1: {proccessed_wikis[0]}\n")
    print(f"Created entries 2: {proccessed_wikis[1]}\n")

    # Write metada.json
    metadata_json = {"items":{"data":proccessed_wikis}}
    with open('../../tests/example_data/wiki_data/metadata.json', 'w') as outfile:
        json.dump(metadata_json, outfile)

if __name__ == '__main__':
    main()
