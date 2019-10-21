import re 

class CMetadata:

    def __init__(self, metadata):
        self.metadata = metadata

    def getAuthor(self):
        try: 
            return self.metadata["data"]["creators"][0]["lastName"] 
        except:
            try:
                return self.metadata["data"]["creators"][0]["name"] 
            except:
                return "no author"

    def getTitle(self):
        try: 
            return self.metadata["data"]["title"]
        except:
            return "no title"

    def getDate(self):
        sDate = ""

        try:
            sDate = self.metadata["data"]["date"]
        except:
            return -1

        match = re.search(".*\\d{3}.*", sDate)
        if match and match.group().isdigit() == True: 
            return int(match.group())
        match = re.search(".*\\d{4}.*", sDate)
        if match and match.group().isdigit() == True:
            return int(match.group())

        return -1

    def getCollections(self):
        collections = []

        try:
            collections = self.metadata["data"]["collections"]
        except:
            return []

        return collections


        
