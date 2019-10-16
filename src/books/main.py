from fuzzywuzzy import fuzz
import json
import book 
import manager 
import search
import searchOptions
import func

bookmanager = manager.CManager()

dict_my = {"jan":1, "Alex":1, "Tobi": 3, "Ferdi":2}
print (sorted((v,k) for(k, v) in dict_my.items()))

'''
sInput = ""
while sInput != "q" :
    sInput = input("Strings: ")
    sSearch = input("Search: ")

    for strs, num in func.extractWords(sInput).items():
        print (strs, num)
        #print (fuzz.ratio(str.lower(), sSearch.lower()))
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
    print (key[1], bookmanager.getBook(key[1]).metadata.getAuthor(), ", ", bookmanager.getBook(key[1]).metadata.getTitle(), ", ", bookmanager.getBook(key[1]).metadata.getDate())
    print (bookmanager.getBook(key[1]).getPreview(searchOpts.word))
    print ("relevance: ", key[0], ", Num Pages: ", bookmanager.getBook(key[1]).numPages)

    '''
    for page, matches in bookmanager.getBook(key).getPages(searchOpts.word, fuzzy).items(): 
        print (page, matches, end=", ")
    print()
    '''
