import os
import metadata
import func

class CBook:

    #Constructor
    def __init__(self, j_metadata):
        self.metadata = metadata.CMetadata(j_metadata)
        self.key=j_metadata["key"]
        self.path = ""
        self.hasOcr = False
        self.mapWordsPages = {}     #key: word, #value: list<int>
        self.mapFuzzyMatches = {}   #key: word, #value: list<string>

        self.mapRelevance = {}
        self.numPages = 0

    
    #Create map of words, called when single book is added
    def createMapOfWords(self, path):
        self.path = path

        if os.path.exists(self.path +"/ocr.txt") == False:
            return

        if os.path.exists(self.path+"/pages_new.txt")==False or os.stat(self.path+"/pages_new.txt").st_size==0 :
            self.createAndSafePages()
        else:
            self.loadPages()
        self.hasOcr = True

    #Create and safe pages on disk
    def createAndSafePages(self):
        print ("starting to create map.")
        read = open(self.path+"/ocr.txt") 

        sBuffer = ""
        pageCounter = 0

        #Create map of words & pages
        for line in read.readlines():

            #If new page, c
            if func.isNewPage(line):

                #Extract words from string and add to dict
                for word, rel in func.extractWords(sBuffer).items():
                    if word not in self.mapWordsPages:
                        self.mapWordsPages[word] = []
                        self.mapRelevance[word]  = float(rel)
                    self.mapWordsPages[word].append(pageCounter) 
                    self.mapRelevance[word] += float(rel)
                pageCounter +=1

                #Create new page as txt
                pagePath = self.path + "/page" + str(pageCounter) +".txt"
                if os.path.exists(pagePath) == True:
                    f = open("page"+str(pageCounter)+".txt", "w")
                    f.write(sBuffer)
                    f.close()
                else:
                    f = open(pagePath, "x")
                    f.write(sBuffer)
                    f.close()

                #Reset buffer
                sBuffer = ""

            #Othewise append line to buffer
            else:
                sBuffer += line
        read.close()

        self.numPages = pageCounter

        #Safe
        f = open(self.path+"/pages_new.txt", "w")
        line = str(self.numPages)+"\n"
        f.write(line)
        for word, pages in self.mapWordsPages.items():
            line = word +";" 
            for page in pages:
                line += str(page) + ","
            relevance = self.mapRelevance[word]
            line += ";" + str(relevance) + "\n"
            f.write(line)


    #Load pages
    def loadPages(self):
        read = open(self.path+"/pages_new.txt")
        print ("loading pages...")

        firstline = read.readline()
        self.numPages = int(firstline)
        for line in read.readlines():
            if len(line) < 2:
                break

            vStrs = line.split(";")
            pages = []
            for page in vStrs[1].split(","):
                pages.append(page)  
            self.mapWordsPages[vStrs[0]] = pages
            self.mapRelevance[vStrs[0]] = float(vStrs[2])
        read.close()

    #Get Pages 
    def getPages(self, sInput, fuzzyness):
        if self.hasOcr == False:
            return {}

        #Full search
        mapPages = {}
        if fuzzyness == 0:
            for page in self.mapWordsPages[sInput]:
                mapPages[page] = [] 
            return mapPages
       
        #Fuzzy search
        for fuzzyMatch in self.mapFuzzyMatches[sInput] :
            for page in self.mapWordsPages[fuzzyMatch] :
                if page not in mapPages :
                    mapPages[page] = [fuzzyMatch]
                else:
                    mapPages[page].append(fuzzyMatch) 

        return mapPages
            
    def getPreview(self, sInput):
        if self.hasOcr == False:
            return "No scans yet. We are trying to change that."

        sInput  = self.getMatch(sInput)
        page    = self.mapWordsPages[sInput][0]
        return self.getPrev(sInput, int(page))

    def getMatch(self, sInput):
        if len(self.mapFuzzyMatches) == 0:
            return sInput
        else:
            return self.mapFuzzyMatches[sInput][0] 

    def getPrev(self, sInput, page):

        #Get page-content from file
        f = open(self.path + "/page" + str(page+1) +".txt")
        sBuffer = f.read();
        f.close()

        #Find match
        pos = sBuffer.lower().find(sInput)

        #If no match found, return error message
        if pos == -1 :
            return "No Preview found, we're sorry for that."

        front = 0
        if pos - front > 75:
            front = pos-75
        back = len(sBuffer)-1
        if back-pos > 75:
            back = pos+75

        sPrev = sBuffer[front:back].replace('\n', '')
        p = sPrev.lower().find(sInput)
        l = len(sInput)

        return "[...] " + sPrev[:p] + "<mark>" + sPrev[p:p+l] + "</mark>" + sPrev[p+l:] + " [...]"
        
        

       

