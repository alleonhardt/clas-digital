function ServerGet(filename, okCallback, errorCallback) {
    let sendRequest = new XMLHttpRequest();
    sendRequest.open("GET", filename, true);
    sendRequest.withCredentials = true;
    sendRequest.onreadystatechange = function() {
	if(sendRequest.readyState == 4) {
	    if(sendRequest.status == 200) {
		okCallback(sendRequest.responseText);
	    } else {
		errorCallback(sendRequest.responseText,sendRequest.status);
	    }
	}
    }
    sendRequest.send(null);

}

let gOCRLoaded = false;
let gHitsLoaded = false;
let gOcrSplittedFile = null;
let gPageLayoutFile = null;

function CorrectedScrollIntoView(elem)
{
    let rect = document.getElementsByClassName("searchbox")[0].getBoundingClientRect();
    window.scrollTo(0,elem.offsetTop-(rect.bottom+50));
}

function HighlightHitsAndConstructLinkList()
{
    let hitlist = document.getElementById("fullsearchhitlist").hitsloaded;

    let ulhitlst = document.getElementById("fullsearchhitlist");
    ulhitlst.innerHTML = "";	//Delete hits if there are any

    if(hitlist && hitlist.books)
    {
	//document.getElementsByClassName("topnav")[0].style.display = "block";
	//document.getElementsByClassName("resizer")[0].style.display = "block";
	let HighlightList = [];

	for(let i = 0; i < hitlist.books.length; i++)
	{
	    let link = document.createElement("a");

	    let inner = hitlist.books[i].page;
	    if(hitlist.is_fuzzy==true)
	    {
		inner+="("+hitlist.books[i].words+"); ";
	    }
	    else
		inner+=" ";
	    link.innerHTML = inner;
	    link.page = hitlist.books[i].page;
	    link.classList.add('hitlinkstyle');
	    link.onclick = function() {
		CorrectedScrollIntoView(document.getElementById("uniquepageid"+hitlist.books[i].page));
		link.style.color = "purple";
	    }

	    ulhitlst.appendChild(link);

	    HighlightList = hitlist.books[i].words.split(",");

	    for(let x = 0; x < HighlightList.length; x++)
	    {
		let thpage = document.getElementById("uniqueocrpage"+hitlist.books[i].page);
		console.log(HighlightList);
		if(thpage!=null)
		    thpage.innerHTML=thpage.innerHTML.replace(new RegExp('('+HighlightList[x].replace(/[-[\]{}()*+?.,\\^$|#\s]/g, '\\$&')+')','gi'),'<mark>$1</mark>');
	    }
	}
    }
    if(location.hash=="")
    {
	if(ulhitlst.firstChild!=null)
	    ulhitlst.firstChild.click();
	else
	{
	    let elem = document.getElementsByClassName("svgpage");
	    if(elem.length>0)
		LoadImageFromSVG(elem[0]);
	}

    }
    else
    {
	if(location.hash=="#page1")
	{
	     let elem = document.getElementsByClassName("svgpage");
	     if(elem.length>0)
		LoadImageFromSVG(elem[0]);
	}
	else
	{
	    CorrectedScrollIntoView(document.getElementById("uniquepageid"+location.hash.substr(5)));
	}
    }
}

function UpdateLinkPrev()
{
    let val = document.getElementsByClassName("hitlinkstyle");
    let page = window.location.hash.substr(5);
    if(page=="")
	page=0;
    else
	page=parseInt(page);
    for(let i = 0; i < val.length; i++)
    {
	if(page<=val[i].page)
	{
	    val[i].scrollIntoView();
	    return;
	}
    }
}

function UpdateViewMode()
{
	let var_arr;
	let var_class = 'pagecontainer';
	let var_class_old = 'pagecontainerFullImg';

	if(document.getElementById("tooglebut").is_full_read)
	{
	    var_arr = isPageAlmostVisible('pagecontainer');
	    var_class = 'pagecontainerFullImg';
	    var_class_old = 'pagecontainer';
	}
	else
	    var_arr = isPageAlmostVisible('pagecontainerFullImg');
	
	for(let i = 0; i < var_arr.length; i++)
	    var_arr[i].classList.replace(var_class_old,var_class);
}

function loadOCRFile(ocrtxt)
{
    ocrtxt = ocrtxt.replace(new RegExp("<",'g'),"&lt;");
    ocrtxt = ocrtxt.replace(new RegExp(">",'g'),"&gt;");

    let RegSrch = /-----\s(\d+)\s.\s\d+\s-----/g;
    let match;
    gOcrSplittedFile = [];
    gOcrSplittedFile.pageNum = [];
    let maxNum = 0;
    while(match = RegSrch.exec(ocrtxt))
    {
	let vl = parseInt(match[1]);
	gOcrSplittedFile.pageNum.push(vl);
	if( vl > maxNum)
	    maxNum = vl;
    }
    gOcrSplittedFile.maxPageNum = maxNum;
    let RegSrchSplit = /-----\s\d+\s.\s\d+\s-----/g;
    gOcrSplittedFile.arr = ocrtxt.split(RegSrchSplit);
    gOcrSplittedFile.arr.shift();
    console.log("SPLIT length: "+gOcrSplittedFile.arr.length);
    //Pagelayout file loaded yet? Then construct the pages
    if(gPageLayoutFile!=null)
	CreatePageLayout();
}

function CreatePageLayout()
{
    console.log("OCR size: "+gOcrSplittedFile.arr.length);
    console.log("Pages size: "+gPageLayoutFile.pages.length);
    for(let i = 0; i < gOcrSplittedFile.arr.length; i++)
    {
	let x = document.createElement("p");
	let cont = document.createElement("div");
	let ocrcont = document.createElement("div");
	let anchor = document.createElement("a");
	anchor.id = "page"+gOcrSplittedFile.pageNum[i];

	ocrcont.classList.add("ocrcontainer");

	cont.classList.add('pagecontainer');
	cont.id = "uniquepageid"+gOcrSplittedFile.pageNum[i];
	x.innerHTML = gOcrSplittedFile.arr[i]+"<br/><b style='float: right;'>"+gOcrSplittedFile.pageNum[i]+"/"+gOcrSplittedFile.maxPageNum+"</b>";
	x.id = "uniqueocrpage"+gOcrSplittedFile.pageNum[i];
	x.classList.add('ocrpage');
	ocrcont.appendChild(anchor);
	ocrcont.appendChild(x);
	cont.appendChild(ocrcont);
	cont.pageNumber = gOcrSplittedFile.pageNum[i];

	document.body.appendChild(cont);
    }

    for(let i = 0; i < gPageLayoutFile.pages.length; i++)
    {
	let doc = document.getElementById("uniquepageid"+gPageLayoutFile.pages[i].pageNumber);
	if(doc==null)
	{
	    console.log("Found page without ocr! At : "+gPageLayoutFile.pages[i].pageNumber);
	    let cont = document.createElement("div");
	    let anchor = document.createElement("a");
	    anchor.id = "page"+gPageLayoutFile.pages[i].pageNumber;


	    cont.classList.add('pagecontainerFullImg');
	    cont.id = "uniquepageid"+gPageLayoutFile.pages[i].pageNumber;
	    cont.pageNumber = gPageLayoutFile.pages[i].pageNumber;

	    document.body.appendChild(cont);
	    doc = cont;
	}

	let svgcode = "<svg class='svgpage' data-width='"+gPageLayoutFile.pages[i].width+"' data-height='"+gPageLayoutFile.pages[i].height+"' data-path='"+"/books/"+getParameterByName('scanId')+ "/" + gPageLayoutFile.pages[i].file.substr(gPageLayoutFile.pages[i].file.search("page"))+"' viewBox='0 0 "+gPageLayoutFile.pages[i].width+" "+gPageLayoutFile.pages[i].height+"'><rect x='0' y='0' width='100%' height='100%'/></svg>";

	doc.innerHTML +=svgcode;
    }

    let timer = null;
    let currentSize = document.getElementsByClassName("searchbox")[0].getBoundingClientRect().bottom;
    document.body.onscroll = function() {
	let gNewSize = document.getElementsByClassName("searchbox")[0].getBoundingClientRect().bottom;
	if(currentSize!=gNewSize)
	{
	    console.log("Resizing ocrs top value!");
	    let lst = document.getElementsByClassName("ocrpage");
	    for(let i = 0; i < lst.length; i++)
		lst[i].style.top = (gNewSize+17)+"px";
	    lst = document.getElementsByClassName("imgpage");
	    for(let i = 0; i < lst.length; i++)
		lst[i].style.top = (gNewSize+17)+"px";
	    currentSize = gNewSize;
	}
	UpdateViewMode();

	if(timer)
	{
	    window.clearTimeout(timer);
	}

	timer = window.setTimeout(function(){
	    let arr = isPageVisible('svgpage');

	    if(window.history.replaceState)
	    {
		let kk = isPageVisible('pagecontainer');
		if(kk.length==0)
		    kk = isPageVisible('pagecontainerFullImg');
		if(kk.length>0)
		{
		    let newurl = window.location.search;
		    newurl+="#page"+kk[0].pageNumber;
		    window.history.replaceState({},null,newurl);
		}
	    }

	    UpdateLinkPrev();
	    for(let i = 0; i < arr.length; i++)
	    {
		LoadImageFromSVG(arr[i]);
	    }
	},400);
    }

    if(gHitsLoaded)
	HighlightHitsAndConstructLinkList();
    gOCRLoaded = true;
}

function LoadImageFromSVG(x)
{
    let img = document.createElement("img");
    img.style.width = x.dataset.width;
    img.style.height = x.dataset.height;
    img.classList.add('imgpage');
    img.src=x.dataset.path;
    img.onload = function(){
	let y = document.body.scrollTop;
	x.parentElement.replaceChild(img,x);
	document.body.scrollTop = y;
    }

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

function isPageAlmostVisible(whatisvis)
{
    let allPages = document.getElementsByClassName(whatisvis);

    let visiblePages = new Array();
    for(let i = 0; i < allPages.length; i++) {
	if(isElementNearViewport(allPages[i])) {
	    visiblePages.push(allPages[i]);
	}
    }
    return(visiblePages);
}

function isElementNearViewport(el) {
    var rect = el.getBoundingClientRect();

    if(rect.bottom <= -2000)
	return(false);
    if(rect.top > (window.innerHeight+2000))
	return(false);
    return(true);

}

function isElementInViewport (el) {
    var rect = el.getBoundingClientRect();
    var optsrect = document.getElementsByClassName("searchbox")[0].getBoundingClientRect();

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
    gOcrSplittedFile = [];

    if(gPageLayoutFile!=null)
	CreatePageLayout();
}

function loadMetadataFile(metadatatxt)
{
    let json = JSON.parse(metadatatxt);
    document.getElementById("bibliography").innerHTML = json.bib;
    if(json.data.creators.length>0)
	document.title = json.data.creators[0].lastName+" ("+json.data.date+"). "+json.data.title;
}

function loadMetadataFileError(errortxt,state)
{
    if(state==403)
	document.getElementById("bibliography").innerHTML = "Sorry this book is copyright protected you cannot view this...";
    else
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
    gPageLayoutFile = {pages:[]};
    if(gOcrSplittedFile!=null)
	CreatePageLayout();
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
    let scanId = getParameterByName('scanId');
    let query = decodeURIComponent(getParameterByName('query')).replace(' ','+');
    let fuzzyness = getParameterByName('fuzzyness');

    ServerGet("/books/"+scanId+"/ocr.txt", loadOCRFile,loadOCRFileError);
    ServerGet("/books/"+scanId+"/info.json",loadMetadataFile,loadMetadataFileError);
    ServerGet("/books/"+scanId+"/readerInfo.json",loadPageLayoutFile,loadMetadataFileError);
    ServerGet("/searchinbook?scanId="+scanId+'&query='+query+'&fuzzyness='+fuzzyness,highlightHitsAndLoadHitlist,highlightHitsAndLoadHitlistError);

    document.getElementById("srchbox").oninput = function() {
	DoFuzzyMatching(this.value);
    }
    document.getElementById('srchbox').addEventListener("focusout", function(event){
	if(event.relatedTarget)
	    event.relatedTarget.click();
	document.getElementById("fuzzysuggestions").style.visibility="hidden";
	document.getElementById("fuzzysuggestions").selec = undefined;});
    document.getElementById("srchbox").addEventListener("keydown",function(event){
	let x = document.getElementById("srchbox").value;
	let k = document.getElementById("fuzzysuggestions").selec;
	if(k==undefined)
	    k = -1;

	if(event.key == "Enter")
	{
	    if(k!=-1)
	    {
		let lst = document.getElementsByClassName("fuzzysugslink");
		lst[k].click();
		return;
	    }
	    doCompleteNewSearch();
	}
	if(event.key=="ArrowUp" || event.key=="ArrowDown")
	{
	    let lst = document.getElementsByClassName("fuzzysugslink");
	    if(k!=-1)
		lst[k].style.background = "";
	    if(event.key=="ArrowUp")
	    {
		k-=1;
		if(k < 0)
		    k = lst.length-1;
	    }
	    else
		k+=1;
	    k = k%lst.length;
	    lst[k].style.background = "#ddd";
	    event.preventDefault();
	    document.getElementById("fuzzysuggestions").selec = k;
	}
    });


    document.addEventListener("fullscreenchange", function (event) {
    if (document.fullscreenElement) {
	document.getElementById("fullbut").src = "/static/GetBooks/fullscreen_exit-24px.svg";
    } else {
	document.getElementById("fullbut").src = "/static/GetBooks/fullscreen-24px.svg";
    }
});
}



function SelectLastHit()
{
    SelectHit(false);
}

function SelectHit(direction)
{
    let curr = window.location.hash;
    let integer = parseInt(curr.substr(5));
    let lnks = document.getElementsByClassName("hitlinkstyle");

    if(direction==false)
    {
	for(let i = lnks.length-1; i > -1; i--)
	{
	    if(parseInt(lnks[i].page) < integer)
	    {
		lnks[i].click();
		return;
	    }
	}
	//We are at or before the first hit, so jump to the last hit
	lnks[lnks.length-1].click();
	return;
    }
    else
    {
	for(let i = 0; i < lnks.length; i++)
	{
	    if(parseInt(lnks[i].page) > integer)
	    {
		lnks[i].click();
		return;
	    }
	}
	//We are at the last hit so jump to the first one
	lnks[0].click();
    }
}

function read_mode(x)
{
    if(x.is_full_read)
    {
	x.is_full_read = false;
	x.src = "/static/GetBooks/chrome_reader_mode-24px.svg";
    }
    else
    {
	x.is_full_read = true;
	x.src = "/static/GetBooks/vertical_split-24px.svg";
    }
    UpdateViewMode();
    CorrectedScrollIntoView(document.getElementById("uniquepageid"+location.hash.substr(5)));
}

function tooglefullscreen(x)
{
    if(x.is_full)
    {
	closeFullscreen(document.documentElement);	
	x.is_full = false;
    }
    else
    {
	openFullscreen(document.documentElement);	
	x.is_full = true;
    }
}


function SelectNextHit()
{
    SelectHit(true);
}

function DoFuzzyMatching(x)
{
    if(x=="")
	return;

    let value = document.getElementsByClassName("ocrpage");
    let results = 0;
    let suggestions = document.getElementById("fuzzysuggestions");
    suggestions.style.visibility = 'visible';
    suggestions.innerHTML = '';

    for(let i = 0; i < value.length; i++)
    {
	let strval = value[i].innerHTML;
	let res = strval.match(new RegExp('.{0,30}('+x+').{0,30}','gi'));
	if(res==null)
	    continue;
	for(let i2 = 0; i2 < res.length; i2++)
	{
	    results+=1;
	    res[i2] = res[i2].replace(new RegExp('('+x+')','gi'),'<mark>$1</mark>');

	    let a = document.createElement("a");
	    a.innerHTML = res[i2]+"<span style='font-weight: bold;float:right;margin-left: 0.8rem;'> S."+(i+1)+"</span>";
	    a.page = i+1;
	    a.word = x;
	    a.result = results-1;
	    a.tabIndex = 0;
	    a.classList.add("fuzzysugslink");
	    a.onclick = function(){
		let page = document.getElementById("uniqueocrpage"+this.page);
		page.innerHTML = page.innerHTML.replace(new RegExp('('+this.word+')','gi'),'<mark>$1</mark>');
		CorrectedScrollIntoView(document.getElementById("uniquepageid"+this.page));
		document.getElementById("fuzzysuggestions").style.visibility = 'hidden';
	    }

	    suggestions.appendChild(a);
	    if(results>9)
	    {
		console.log(results);
		return;
	    }
	}
    }
    if(results==0)
	suggestions.style.visibility = 'hidden';

}

function doCompleteNewSearch()
{
    let newurl = "/GetBooks.html?query="+document.getElementById("srchbox").value+"&scanId="+getParameterByName("scanId")+"&fuzzyness="+getParameterByName('fuzzyness');
    window.location = newurl;
}

window.addEventListener("load",initialise,false);
