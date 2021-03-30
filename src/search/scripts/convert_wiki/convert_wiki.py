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

    # Load random names
    first_names = []
    sir_names = []
    with open('random_names.txt') as names:
        line = names.readline()
        while line:
            first_names.append(line.split(" ")[0])
            sir_names.append(line.split(" ")[1])
            line = names.readline()
    print(f"Parsed {len(first_names)} first names and {len(sir_names)} sir names.\n")

    # Load random cities.
    cities = []
    with open('random_cities.txt') as ran_cities:
        line = ran_cities.readline()
        while line:
            cities.append(line)
            line = ran_cities.readline()
    print(f"Parsed {len(cities)} random cities.\n")


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
        new_entry["data"]["date"] = str(random.randint(1800,2020))

        ran_keys += 1
        new_entry["key"] = str(ran_keys)
        new_entry["data"]["key"] = new_entry["key"]

        a_first_name = first_names[random.randint(0, len(first_names)-1)]
        a_last_name = sir_names[random.randint(0, len(sir_names)-1)]
        e_first_name = first_names[random.randint(0, len(first_names)-1)]
        e_last_name = sir_names[random.randint(0, len(sir_names)-1)]
        new_entry["data"]["creators"][0]["firstName"] = a_first_name
        new_entry["data"]["creators"][0]["lastName"] = a_last_name
        new_entry["data"]["creators"][1]["firstName"] = e_first_name
        new_entry["data"]["creators"][1]["lastName"] = e_last_name

        new_entry["data"]["title"] = wiki_json["title"]
        new_entry["data"]["place"] = cities[random.randint(0, len(cities)-1)]

        new_entry["bib"] = (f'{a_last_name}, {a_first_name}: '
            + f'{new_entry["data"]["title"]}, {new_entry["data"]["place"]}, '
            + f'{new_entry["data"]["date"]}.')
        new_entry["citation"] = new_entry["bib"] 
        new_entry["rights"] = "CLASfrei"
        proccessed_wikis.append(new_entry)

        # Create book directory and ocr
        path = "../../tests/example_data/wiki_data/test_books/" + str(new_entry["key"])
        os.mkdir(path)
        f = open(path+"/ocr.txt", "a")
        f.write(wiki_json["body"])
        f.close()

    print(f"Created entries 1: {proccessed_wikis[0]}\n")
    print(f"Created entries 1: {proccessed_wikis[1]}\n")

    # Write metada.json
    metadata_json = {"items":{"data":proccessed_wikis}}
    with open('../../tests/example_data/wiki_data/metadata.json', 'w') as outfile:
        json.dump(metadata_json, outfile)

if __name__ == '__main__':
    main()
