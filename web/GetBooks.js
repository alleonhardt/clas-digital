class Server {
	static Get(filename, okCallback, errorCallback) {
		let sendRequest = new XMLHttpRequest();
		sendRequest.open("GET", filename, true);
		sendRequest.onreadystatechange = () => {
			if(sendRequest.readyState == 4) {
				if(sendRequest.status == 200) {
					okCallback(sendRequest.responseText);
				} else {
					errorCallback(sendRequest.responseText);
				}
			}
		}
		sendRequest.send(null);

	}
}

let gOCRLoaded = false;
let gHitsLoaded = false;
let gOcrSplittedFile = null;
let gPageLayoutFile = null;

function HighlightHitsAndConstructLinkList()
{
    let hitlist = document.getElementById("fullsearchhitlist").hitsloaded;

    let ulhitlst = document.getElementById("fullsearchhitlist");
    ulhitlst.innerHTML = "";	//Delete hits if there are any

    if(hitlist && hitlist.books)
    {
	let HighlightList = [];
	if(hitlist.is_fuzzy==false)
	    HighlightList = getParameterByName('query').split('+');

	for(let i = 0; i < hitlist.books.length; i++)
	{
	    let lst = document.createElement("li");
	    let link = document.createElement("a");
	    
	    let inner = hitlist.books[i].page;
	    if(hitlist.is_fuzzy==true)
	    {
		inner+="("+hitlist.books[i].words+");";
	    }
	    link.innerHTML = inner;
	    link.page = hitlist.books[i].page;
	    link.onclick = function() {
		document.getElementById("page"+hitlist.books[i].page).scrollIntoView();
	    }

	    lst.appendChild(link);
	    ulhitlst.appendChild(lst);

	    if(hitlist.is_fuzzy)
	    {
		HighlightList = hitlist.books[i].words.split(",");
	    }

	    for(let x = 0; x < HighlightList.length; x++)
	    {
		let thpage = document.getElementById("uniqueocrpage"+hitlist.books[i].page);
		if(thpage!=null)
		    thpage.innerHTML=thpage.innerHTML.replace(new RegExp('('+HighlightList[x].replace(/[-[\]{}()*+?.,\\^$|#\s]/g, '\\$&')+')','gi'),'<mark>$1</mark>');
	    }
	    
	}
    }
    if(location.hash=="")
    {
	document.getElementById("page"+hitlist.books[0].page).scrollIntoView();
    }
    else
    {
	document.getElementById("page"+location.hash.substr(5)).scrollIntoView();
    }
}

function loadOCRFile(ocrtxt)
{
    ocrtxt = ocrtxt.replace(new RegExp("<",'g'),"&lt;");
    ocrtxt = ocrtxt.replace(new RegExp(">",'g'),"&gt;");

    let RegSrch = /-----\s\d+\s.\s\d+\s-----/g;
    gOcrSplittedFile = ocrtxt.split(RegSrch);
    console.log("SPLIT length: "+gOcrSplittedFile.length);
    //Pagelayout file loaded yet? Then construct the pages
    if(gPageLayoutFile!=null)
	CreatePageLayout();
}

function CreatePageLayout()
{
    console.log("OCR size: "+gOcrSplittedFile.length);
    console.log("Pages size: "+gPageLayoutFile.pages.length);
    for(let i = 0; i < gOcrSplittedFile.length; i++)
    {
	let x = document.createElement("p");
	let cont = document.createElement("div");
	let ocrcont = document.createElement("div");
	let anchor = document.createElement("a");
	anchor.id = "page"+(i+1);

	ocrcont.classList.add("ocrcontainer");
	
	cont.classList.add('pagecontainer');
	cont.id = "uniquepageid"+(i+1);
	x.innerHTML = gOcrSplittedFile[i]+"<br/><b style='float: right;'>"+(i+1)+"/"+gOcrSplittedFile.length+"</b>";
	x.id = "uniqueocrpage"+(i+1);
	x.classList.add('ocrpage');
	ocrcont.appendChild(anchor);
	ocrcont.appendChild(x);
	cont.appendChild(ocrcont);
	cont.pageNumber = (i+1);

	if(gPageLayoutFile.pages.length>i)
	{
	    //let svgcode = document.createElement("svg");
	    let svgcode = "<svg class='svgpage' data-width='"+gPageLayoutFile.pages[i].width+"' data-height='"+gPageLayoutFile.pages[i].height+"' data-path='"+"/books/"+getParameterByName('scanId')+ "/" + gPageLayoutFile.pages[i].file.substr(gPageLayoutFile.pages[i].file.search("page"))+"' viewBox='0 0 "+gPageLayoutFile.pages[i].width+" "+gPageLayoutFile.pages[i].height+"'><rect x='0' y='0' width='100%' height='100%'/></svg>";

	    cont.innerHTML +=svgcode;
	}
	document.getElementsByClassName("botpage")[0].appendChild(cont);
    }

    let timer = null;
    document.getElementsByClassName("botpage")[0].onscroll = function() {
	if(timer)
	{
	    window.clearTimeout(timer);
	}
	
	timer = window.setTimeout(function(){
	let arr = isPageVisible('svgpage');
	
	if(window.history.replaceState)
	{
	    let kk = isPageVisible('pagecontainer');
	    if(kk.length>0)
	    {
		console.log(kk);
		console.log(kk[0].pageNumber);
		let newurl = window.location.search;
		newurl+="#page"+kk[0].pageNumber;
		window.history.replaceState({},null,newurl);
	    }
	}

	for(let i = 0; i < arr.length; i++)
	{
	    let img = document.createElement("img");
	    img.style.width = arr[i].dataset.width;
	    img.style.height = arr[i].dataset.height;
	    img.classList.add('imgpage');
	    img.src=arr[i].dataset.path;
	    img.onload = function(){
		let x = document.getElementsByClassName("botpage")[0].scrollTop;
		arr[i].parentElement.replaceChild(img,arr[i]);
		document.getElementsByClassName("botpage")[0].scrollTop = x;
	    }
	}
	},400);
    }

    if(gHitsLoaded)
	HighlightHitsAndConstructLinkList();
    gOCRLoaded = true;
}

function isPageVisible(whatisvis)
{
    let allPages = document.getElementsByClassName(whatisvis);

    let visiblePages = new Array();
    for(let i = 0; i < allPages.length; i++) {
	    if(isElementInViewport(allPages[i])) {
		    visiblePages.push(allPages[i]);
	    }
    }
    return(visiblePages);
}

function isElementInViewport (el) {
	var rect = el.getBoundingClientRect();
	var optsrect = document.getElementsByClassName("topnav")[0].getBoundingClientRect();
	
	// it's definitely outside if bottom < 0
	if(rect.bottom <= 0)
		return(false);
	if(rect.top > window.innerHeight)
		return(false);
	if(rect.bottom<(optsrect.bottom+90))
		return(false);
	return(true);
}


function loadOCRFileError(errortext)
{
    document.getElementById("uniqueocrpage0").innerHTML = "Could not load ocr file sorry for this :(";
}

function loadMetadataFile(metadatatxt)
{
    let json = JSON.parse(metadatatxt);
    document.getElementById("bibliography").innerHTML = json.bib;
    if(json.data.creators.length>0)
		    document.title = json.data.creators[0].lastName+" ("+json.data.date+"). "+json.data.title;
}

function loadMetadataFileError(errortxt)
{
    document.getElementById("bibliography").innerHTML = "Could not load metadata sorry for that :(";
}

function loadPageLayoutFile(layout)
{   
    gPageLayoutFile = JSON.parse(layout);
    //OCR loaded and splitted yet? Then start building the page!
    if(gOcrSplittedFile!=null)
	CreatePageLayout();
}

function loadPageLayoutFileError(errortxt)
{
    document.getElementById("bookpageimagcontainer").innerHTML = "Could not load image layout sorry for that :(";
}

function highlightHitsAndLoadHitlistError(text)
{
    document.getElementById("fullsearchhitlist").innerHTML = "<li>Could not load hit list sorry for that</li>";
    document.getElementById("fullsearchhitlist").hitsloaded = null;
    if(gOCRLoaded)
	HighlightHitsAndConstructLinkList();
    gHitsLoaded = true;
}

function highlightHitsAndLoadHitlist(hits)
{
    let searchhits = JSON.parse(hits);
    document.getElementById("fullsearchhitlist").hitsloaded = searchhits;
    if(gOCRLoaded)
	HighlightHitsAndConstructLinkList();
    gHitsLoaded = true;
}

function initialise()
{
    document.getElementById("srchButton").value = getParameterByName('query');
    let scanId = getParameterByName('scanId');
    let query = getParameterByName('query');
    let fuzzyness = getParameterByName('fuzzyness');

    Server.Get("/books/"+scanId+"/ocr.txt", loadOCRFile,loadOCRFileError);
    Server.Get("/books/"+scanId+"/info.json",loadMetadataFile,loadMetadataFileError);
    Server.Get("/books/"+scanId+"/readerInfo.json",loadPageLayoutFile,loadMetadataFileError);
    Server.Get("/searchinbook?scanId="+scanId+'&query='+encodeURIComponent(query)+'&fuzzyness='+fuzzyness,highlightHitsAndLoadHitlist,highlightHitsAndLoadHitlistError);
}
