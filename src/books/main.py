from fuzzywuzzy import fuzz
import json
import book 
import manager 
import search
import searchOptions
import func

bookmanager = manager.CManager()

language = {'ä': 'a', 'ö':'o', 'ü':'u', 'Ä':'A', 'Ö':'O', 'Ü':'U', 'è':'e', 'é':'e'}

sInput = ""
while sInput != "q" :
    sInput = input("Strings: ")
    sSearch = input("Search: ")
    for char in sSearch:
        if char in language:
            sSearch= sSearch.replace(char, language[char])

    for strs, num in func.extractWords(sInput).items():
        '''
        for char in strs:
            if char in language:
                strs = strs.replace(char, language[char])
        '''
        print (strs, sSearch, fuzz.partial_ratio(strs.lower(), sSearch.lower()))

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

    for page, matches in bookmanager.getBook(key).getPages(searchOpts.word, fuzzy).items(): 
        print (page, matches, end=", ")
    print()
'''
