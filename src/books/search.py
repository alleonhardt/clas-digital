
from fuzzywuzzy import fuzz
import searchOptions

class CSearch:
    def __init__ (self, sOpts, searchID):
        self.searchOpts = sOpts
        self.id = searchID
        self.searchResults = {}

    #Search functions, calls specific search
    def search(self, mapWords, mapWordsTitle, mapBooks):

        #Full search
        if self.searchOpts.fuzzyness == 0:
            if self.searchOpts.onlyTitle == False:
                self.normalSearch(mapWords)              #search in ocr
            if self.searchOpts.onlyOcr == False:
                self.normalSearch(mapWordsTitle)         #search in title

        #Fuzzy search
        elif self.searchOpts.fuzzyness == 1:
            if self.searchOpts.onlyTitle == False:
                self.fuzzySearch(mapWords)          #search in ocr
            if self.searchOpts.onlyOcr == False:
                self.fuzzySearch(mapWordsTitle)          #search in title
            
        self.authorSearch(mapBooks)


        return self.searchResults   #return results


    #Finds only full matches (O(1))
    def normalSearch(self, mapWords):
        print("checking for: ", self.searchOpts.word)
        if self.searchOpts.word in mapWords: 
            self.searchResults.update(mapWords[self.searchOpts.word])

    #Find fuzzy matches (not implemented yet - O(n*fuzzysearch)
    def fuzzySearch(self, mapWords):
        print("Cheking fuzzy for: ", self.searchOpts.word)
        for word, res in mapWords.items():
            if fuzz.ratio(word, self.searchOpts.word) >= 85 :
                self.searchResults.update(res)
                for key, book in res.items():
                    if self.searchOpts.word not in book.mapFuzzyMatches:
                        book.mapFuzzyMatches[self.searchOpts.word] = []
                    book.mapFuzzyMatches[self.searchOpts.word].append(word) 
                    

    #Find author
    def authorSearch(self, mapBooks):
        for key, book in mapBooks.items() :
            if book.metadata.getAuthor().lower() == self.searchOpts.word :
                self.searchResults[key] = book



         



