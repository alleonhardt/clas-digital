import json
import book 
import manager 
import search
import searchOptions
import func

bookmanager = manager.CManager()

'''
sInput = ""
while sInput != "q" :
    sInput = input("Strings: ")
    sSearch = input("Search: ")

    for str in func.extractWords(sInput):
        print (fuzz.partial_ratio(str.lower(), sSearch.lower()))

''' 

with open("zotero.json") as read_file:
    data = json.load(read_file)

bookmanager.updateZotero(data)
bookmanager.initialize()

word = input("Search for: ")
fuzzy = int(input("Fuzzyness: "))

searchOpts = searchOptions.CSearchOptions(word.lower(), fuzzy, False, True)
search = search.CSearch(searchOpts, 0)
bookmanager.addSearch(search)

results = bookmanager.search(0)
for key, book in results.items():
    print (book.metadata.getAuthor(), ", ", book.metadata.getTitle(), ", ", book.metadata.getDate())
    pages = book.getPages(searchOpts.word, fuzzy)
    for page in pages: 
        print (page)
        print ("Matches: ", pages[page])

'''

for key, book in bookmanager.search(0).items():
    print (book.metadata.getAuthor(), ", ", book.metadata.getTitle(), ", ", book.metadata.getDate())
    if len(book.getPages(searchOpts.word, fuzzy)) > 0:
        for page, matches in book.getPages(searchOpts.word, fuzzy).items():
            print (page, matches)
    else:
        print ("No pages found!")
'''
