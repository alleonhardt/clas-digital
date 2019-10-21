from fuzzywuzzy import fuzz
import json
import book 
import manager 
import search
import searchOptions
import func
import time
from colorama import Fore

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


with open("../../bin/zotero.json") as read_file:
    data = json.load(read_file)


bookmanager.updateZotero(data)
bookmanager.initialize()

word = ""
counter =0
while word != ":q":
    word = input("Search for: ")
    fuzzy = int(input("Fuzzyness: "))

    cols = ["RFWJC42V", "XCFFDRQC", "RBB8DW5B", "WIXP3DS3"]
    searchOpts = searchOptions.CSearchOptions(word.lower(), fuzzy, False, True, "", 0, 2018,cols)
    mySearch = search.CSearch(searchOpts, counter)
    bookmanager.addSearch(mySearch)
    counter+=1

    start = time.time()
    results = bookmanager.search(0)
    '''
    for key in results:
        print (key[1], bookmanager.getBook(key[1]).metadata.getAuthor(), ", ", bookmanager.getBook(key[1]).metadata.getTitle(), ", ", bookmanager.getBook(key[1]).metadata.getDate())
        print (Fore.GREEN + bookmanager.getBook(key[1]).getPreview(searchOpts.word))
        print (Fore.BLUE + "Relevance: ", key[0], Fore.RESET)
    '''
    end = time.time()
    print (Fore.RED, len(results), " in ", end-start, " secs.", Fore.RESET)

