from fuzzywuzzy import fuzz
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
        print (fuzz.ratio(str.lower(), sSearch.lower()))

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
for key in results:
    print (bookmanager.getBook(key).metadata.getAuthor(), ", ", bookmanager.getBook(key).metadata.getTitle(), ", ", bookmanager.getBook(key).metadata.getDate())
    print ("Relevance: ", bookmanager.getBook(key).getRelevance(searchOpts.word, fuzzy))

    '''
    pages = book.getPages(searchOpts.word, fuzzy)
    print (type(pages))
    for page, matches in pages.items(): 
        print (page, matches, end=", ")
    print()
    '''

