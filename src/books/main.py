from fuzzywuzzy import fuzz
import json
import book 
import manager 
import search
import searchOptions
import func

bookmanager = manager.CManager()


''' language = {'ä': 'a', 'ö':'o', 'ü':'u', 'Ä':'A', 'Ö':'O', 'Ü':'U', 'è':'e', 'é':'e'}'''

'''
f = open("test.txt")
sInput = f.read();
f.close()

for strs, num in func.extractWords(sInput).items():
#    for char in strs:
#        if char in language:
#            strs = strs.replace(char, language[char])
    print (strs)
#   print (strs, sSearch, fuzz.partial_ratio(strs.lower(), sSearch.lower()))

'''


with open("zotero.json") as read_file:
    data = json.load(read_file)


bookmanager.updateZotero(data)
bookmanager.initialize()

word = input("Search for: ")
fuzzy = int(input("Fuzzyness: "))

cols = ["RFWJC42V", "XCFFDRQC", "RBB8DW5B", "WIXP3DS3"]
searchOpts = searchOptions.CSearchOptions(word.lower(), fuzzy, False, True, "", 0, 2018,cols)
search = search.CSearch(searchOpts, 0)
bookmanager.addSearch(search)

results = bookmanager.search(0)
for key in results:
    print (key[1], bookmanager.getBook(key[1]).metadata.getAuthor(), ", ", bookmanager.getBook(key[1]).metadata.getTitle(), ", ", bookmanager.getBook(key[1]).metadata.getDate())
    print (bookmanager.getBook(key[1]).getPreview(searchOpts.word))
    print ("relevance: ", key[0], ", Num Pages: ", bookmanager.getBook(key[1]).numPages)

    for page, matches in bookmanager.getBook(key[1]).getPages(searchOpts.word, fuzzy).items(): 
        print (page, matches, end=", ")
    print()
