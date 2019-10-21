
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
                self.fuzzySearch(mapWords, mapBooks)          #search in ocr
            if self.searchOpts.onlyOcr == False:
                self.fuzzySearch(mapWordsTitle, mapBooks)          #search in title
            
        #Search for author
        if self.searchOpts.onlyOcr == False:
            self.authorSearch(mapBooks)

        return {k:self.searchResults[k] for k in self.searchResults if self.checkSearchOptions(mapBooks[k]) == True}


    #Finds only full matches (O(1))
    def normalSearch(self, mapWords):
        print("checking for: ", self.searchOpts.word)
        if self.searchOpts.word in mapWords: 
            self.searchResults.update(mapWords[self.searchOpts.word])


    #Find fuzzy matches (not implemented yet - O(n*fuzzysearch)
    def fuzzySearch(self, mapWords, mapBooks):

        print("Cheking fuzzy for: ", self.searchOpts.word)

        #Iterating over all words in map
        for word, res in mapWords.items():

            #If Match, add match to results
            ratio = fuzz.ratio(word, self.searchOpts.word)
            if ratio >= 89 :
                self.myUpdate(self.searchResults, res, ratio)

                #Add match to list of words matching searched word
                for key, rel in res.items():
                    if self.searchOpts.word not in mapBooks[key].mapFuzzyMatches:
                        mapBooks[key].mapFuzzyMatches[self.searchOpts.word] = []
                    mapBooks[key].mapFuzzyMatches[self.searchOpts.word].append(word) 
                    

    #Find author
    def authorSearch(self, mapBooks):
        for key, book in mapBooks.items() :
            if book.metadata.getAuthor().lower() == self.searchOpts.word :
                self.searchResults[key] = book

    def myUpdate(selfd, results1, results2, ratio):
        for key, relevance in results2.items() :
            if key in results1:
                results1[key] += relevance*(ratio/100)
            else:
                results1[key] = relevance

    def checkSearchOptions(self, book):

        metadata = book.metadata
        #Check author
        if self.searchOpts.author != "" and self.searchOpts.author != metadata.getAuthor():
            return False

        #Check date
        if metadata.getDate() < self.searchOpts.dFrom or metadata.getDate() > self.searchOpts.dTo:
            return False

        exists = False
        for col in metadata.getCollections():
            if col in self.searchOpts.collections:
                exists = True
        
        return exists 
            
        



         



