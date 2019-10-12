import json
import book 
import manager 

bookmanager = manager.CManager()

with open("zotero.json") as read_file:
    data = json.load(read_file)
bookmanager.updateZotero(data)

mapBooks = bookmanager.mapBooks;

counter = 0
noAutor = 0
for k, v in mapBooks.items():
    if v.getAuthor() == "no author":
        noAutor += 1
    counter += 1

print(counter, noAutor)
    
