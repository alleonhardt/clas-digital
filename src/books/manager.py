import os
import book
import search
import func

class CManager:

    #Constructor
    def __init__ (self):
        self.mapBooks = {}          #key: string(key), value: book(CBook)
        self.mapWords = {}          #key: string(word, value: {string(key), book(Book)}
        self.mapWordsTitle = {}
        self.mapSearch = {}

    #Get book
    def getBook(self, key):
        if key in self.mapBooks:
            return self.mapBooks[key]
        return 

    #Update zotero: updates all books in zotero and metadata
    def updateZotero(self, j_items):
        for it in j_items: 
            myBook = book.CBook(it)
            self.mapBooks[it["key"]] = myBook


    #Addes book from disk. Only books already in map will be loaded
    def initialize(self):

        #Iterate over all folders
        for subdir in os.listdir("../../web/books"):
            if subdir in self.mapBooks:
                self.addBook(subdir)
            else:
                print("book found that isn't in map")

        #Create map of all words
        self.createMapWords()
        self.createMapWordsTitle()
        print("Map of words: ", len(self.mapWords))
        print("Map if Title: ", len(self.mapWordsTitle))


    #Addes a single book
    def addBook(self, key):

        ##Check if path exists
        if os.path.exists("../../web/books/" + key) == False:
            return 

        #Create "physical" book, with ocr.
        self.mapBooks[key].createMapOfWords("../../web/books/" + key)


    #Addes new search
    def addSearch(self, search):
        self.mapSearch[search.id] = search


    #Creates "big" map of word, containing all words from all book (key) and 
    #  a list of book containing the word (value)
    def createMapWords(self):
        for key, book in self.mapBooks.items():
            self.storeWords(self.mapWords, book.mapWordsPages, key, book)
    #Creates "big" map of word, containing all words from all booktitles (key) and 
    #  a list of book containing the word (value)
    def createMapWordsTitle(self):
        for key, book in self.mapBooks.items():
            self.storeWords(self.mapWordsTitle, func.extractWords(book.metadata.getTitle()), key, book)
    #store words
    def storeWords(self, mapWords, mapFrom, key, book):
        for word, pages in mapFrom.items():
            if word not in mapWords:
                mapWords[word] = {}
            mapWords[word][key] = book


    #Main search function callig searchfunction from CSearch
    def search(self, searchID):

        #Check whether given search exists
        if searchID in self.mapSearch == False:
            return {}

        #Get search
        search = self.mapSearch[searchID]

        #start normal search
        word = search.searchOpts.word
        fuzzyness = search.searchOpts.fuzzyness
        results = search.search(self.mapWords, self.mapWordsTitle, self.mapBooks)

        return list(map(lambda x:x[1], sorted((v.getRelevance(word, fuzzyness)*(-1),k) for(k, v) in results.items())))
            
            




        
        
            
        
