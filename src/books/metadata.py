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
        date = ""
        try:
            date = self.metadata["data"]["date"]
        except:
            return "not date"

        match = re.search(".*\\d{3}.*", date)
        if match and match.group().isdigit() == True: 
            return int(match.group())
        match = re.search(".*\\d{4}.*", date)
        if match and match.group().isdigit() == True:
            return int(match.group())


        
