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

let gUpdateInterval = 400;
class PageLoader {
	constructor() {
		this.queue = new Array();
	}


	queuePage(pageNode) {
		console.log("queuePage called for: " + pageNode.info);
		if(this.queue.length > 0) {
			this.queue.unshift(pageNode);
		} else {
			this.loadPage(pageNode);
		}
	}

	checkQueue() {
		if(this.queue.length > 0) {
			this.loadPage(queue.shift());
		}

		if(this.queue.length > 5)
			this.queue.splice(5);
	}

	deferedCheckQueue() {
		if(this.queue.length > 0)
			window.setTimeout(this.checkQueue.bind(this), 50);
	}

	loadPage(pageNode) {
		console.log("loadPage called for: " + pageNode.info);

		pageNode.classList.remove("placeholder");
		let img = document.createElement("img");
		img.src = pageNode.scanFilePath;
		img.id = "uniqueImageId"+(pageNode.info+1);
		img.classList = "findImg";
		img.page = pageNode.info;
		img.onload = function() {
			let scrollPosition = document.getElementById("thepagereader").scrollTop;
			pageNode.replaceChild(img, pageNode.querySelector("svg"));
			document.getElementById("thepagereader").scrollTop = scrollPosition; //Restore scroll position
			this.deferedCheckQueue();
		}.bind(this);
		img.onerror = function() {
			if(pageNode.first_try ===undefined)
			{
				pageNode.scanFilePath = "/404.jpeg";
				pageNode.first_try = false;
				this.loadPage.bind(this)(pageNode);
			}
		}.bind(this);
	}
}

let pageLoader = new PageLoader(); 
let pageVal = [];
function getVisiblePages()
{
	let allPages = document.querySelectorAll("div.page");

	let visiblePages = new Array();
	for(let i = 0; i < allPages.length; i++) {
		if(isElementInViewport(allPages[i])) {
			visiblePages.push(allPages[i]);
		}
	}
	return(visiblePages);
}

function getVisiblePagesOCR()
{
	let allPages = document.getElementsByClassName("ocrpage");
	let visiblePages = new Array();
	for(let i = 0; i < allPages.length; i++) {
		if(isElementInViewport2(allPages[i])) {
		    visiblePages.push(allPages[i]);
		}
	}

	console.log(visiblePages[0]);
	return(visiblePages);
}


function updateAfterScroll() {
	let visiblePages = getVisiblePages();

	for(let i = 0; i < visiblePages.length; i++) {
		if(visiblePages[i].classList.contains("placeholder"))
			pageLoader.queuePage(visiblePages[i]);
	}

	if(visiblePages.length > 0 && 
			visiblePages[visiblePages.length - 1].nextElementSibling != null &&
			visiblePages[visiblePages.length - 1].nextElementSibling.classList.contains("placeholder"))
	{
		pageLoader.queuePage(visiblePages[visiblePages.length - 1].nextElementSibling);
	}
}

function isElementInViewport (el) {
	var rect = el.getBoundingClientRect();
	
	// it's definitely outside if bottom < 0
	if(rect.bottom <= 0)
		return(false);
	if(rect.top > window.innerHeight)
		return(false);

	return(true);
}

function isElementInViewport2(el) {
	var rect = el.getBoundingClientRect();
	var optsrect = document.getElementById("SearchOpts").getBoundingClientRect();
	
	// it's definitely outside if bottom < 0
	if(rect.bottom <= 0)
		return(false);
	if(rect.top > document.getElementById("OCRBuffer").clientHeight)
		return(false);
	if(rect.bottom<(optsrect.bottom+90))
		return(false);
	return(true);

}

function parseURLParams(url) {
	let parser = document.createElement('a');
	parser.href = url;

	let searchObject = {};
	let queries = parser.search.replace(/^\?/, '').split('&');
	for(let i = 0; i < queries.length; i++ ) {
		let split = queries[i].split('=');
		searchObject[split[0]] = split[1];
	}
	
	return(searchObject);
}

function doAdditionalSearch() {
	let parsedUrlParams = parseURLParams(window.location.href);
	window.open("/GetBooks?query="+document.getElementById("adSrch").value+"&scanId="+parsedUrlParams['scanId']+"&fuzzyness="+parsedUrlParams['fuzzyness']);

}


function GetOCRTxt() {
				return new Promise(function(resolve,reject){
				
			    let parsedUrlParams = parseURLParams(window.location.href);
				Server.Get("/books/"+parsedUrlParams['scanId']+"/ocr.txt",function(content) {
			    
				content = content.replace(new RegExp("<",'g'),"&lt;");
				content = content.replace(new RegExp(">",'g'),"&gt;");

				let RegSrch = /-----\s(\d+)\s.\s(\d+)\s-----/g;
				content = '<p data-page="1" id="uniqueocrpage0" class="ocrpage">' + content;
				content = content.replace(RegSrch,'</p><p data-page="$1" id="uniqueocrpage$1" class="ocrpage" ondblclick="SwitchToPage(this);"> Page $1 / $2 <br/>');
				content+="</p>";	
				document.getElementById("OCRBuffer").innerHTML = content;
				if(location.hash!="")
				{
				    document.getElementById("uniqueocrpage"+location.hash.substr(5)).scrollIntoView();
				}
				resolve(10);

							},function(content){document.getElementById("OCRBuffer").innerHTML = "Could not load ocr file, sorry for that...";});
							});
}

function SwitchToPage(x)
{
    let val = document.getElementById("uniqueDivId"+x.dataset.page);
    let vspg = getVisiblePages();
    let fnc = this.onscroll;
    this.onscroll = null;
    if(vspg.indexOf(val)==-1)
    {
	if(val != null)
	    val.scrollIntoView();
	else
	    document.getElementById("uniquePageNode"+x.dataset.page).scrollIntoView();
    if(window.history.replaceState)
    {
	let newurl = window.location.search;
	newurl+="#page"+x.dataset.page;
	window.history.replaceState({},null,newurl);
    }
    }
    this.onscroll = fnc;
}


let metadata = {};

let globalScrollLock = false;
function getParameterByName(name, url) {
//COPYRIGHT TO https://stackoverflow.com/questions/901115/how-can-i-get-query-string-values-in-javascript
	if (!url) url = window.location.href;
    name = name.replace(/[\[\]]/g, '\\$&');
    var regex = new RegExp('[?&]' + name + '(=([^&#]*)|&|#|$)'),
        results = regex.exec(url);
    if (!results) return null;
    if (!results[2]) return '';
    return decodeURIComponent(results[2].replace(/\+/g, ' '));
}

function UpdateLinkPrev()
{
	    let val = document.getElementsByClassName("coollnklist");
	    let page = window.location.hash.substr(5);
	    if(page=="")
		page=0;
	    else
		page=parseInt(page);
	    for(let i = 0; i < val.length; i++)
	    {
		if(page<=val[i].dataset.page)
		{
		    val[i].scrollIntoView();
		    val[i].scrollLeft += "100";
		    if(document.getElementById("SearchOpts").classList.contains("secondoptioncontlay"))
			document.getElementById("SearchOpts").scrollTop = 0;
		    return;
		}

	    }
}

function triggerDblclick(x)
{
    var event = new MouseEvent('dblclick', {
    'view': window,
    'bubbles': true,
    'cancelable': true
  });
    x.dispatchEvent(event);
}

function srchbarlosefocus()
{
										    
    document.getElementById("fuzzysuggestions").style.display = 'none';
}

async function DoFuzzyMatching(x)
{
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
	    a.innerHTML = res[i2]+"<span style='font-weight: bold;float:right;margin-left: 0.8rem;'> S."+i+"</span>";
	    a.page = i;
	    a.word = x;
	    a.result = results-1;
	    a.tabIndex = 0;
	    a.classList.add("fuzzysugslink");
	    a.onclick = function(){
		let page = document.getElementById("uniqueocrpage"+this.page);
		page.innerHTML = page.innerHTML.replace(new RegExp('('+this.word+')','gi'),'<mark>$1</mark>');
		page.scrollIntoView();
		document.getElementById("fuzzysuggestions").style.visibility = 'hidden';
	    }

	    a.onmouseover = function() {
		    let sugg = document.getElementById("fuzzysuggestions");
		    let lnks = document.getElementsByClassName("fuzzysugslink");
		    let value = sugg.selec;
		    if(value!=undefined)
			lnks[value].style.background = "";
		    a.style.background = "#ddd";
		    sugg.selec = a.result;
	    }

	    suggestions.appendChild(a);
	    if(results>9)
	    {
		console.log(results);
		return;
	    }
	}
    }
}

function doCompleteNewSearch()
{
    let newurl = "/GetBooks.html?query="+document.getElementById("srchbox").value+"&scanId="+getParameterByName("scanId")+"&fuzzyness="+getParameterByName('fuzzyness');
    window.location = newurl;
}

let gCurrentFocus = 'ocr';
let gCurrentTimer;

function sync_pages()
{
	    let val;
	    if(gCurrentFocus=='ocr')
		val = getVisiblePagesOCR();
	    else
		val = getVisiblePages();
	    triggerDblclick(val[0])
	    UpdateLinkPrev();
	    updateAfterScroll();
}


function init() {
	    document.getElementById("srchbox").innerHTML = getParameterByName('query');
	    document.getElementById("srchbox").addEventListener("input",function(event) {
		document.getElementById("fuzzysuggestions").selec = undefined;
		if(document.getElementById("srchbox").value=="")
		    return;
		DoFuzzyMatching(document.getElementById("srchbox").value);
	    });

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
    document.getElementById('searchboxstuff').addEventListener("focusout", function(event){if(event.relatedTarget)event.relatedTarget.click();console.log(event.relatedTarget);document.getElementById("fuzzysuggestions").style.visibility="hidden";document.getElementById("fuzzysuggestions").selec = undefined;});
    
    document.getElementById("thepagereader").onscroll = function() {
	    gCurrentFocus = 'page';
	    if(gCurrentTimer)
	    {
		window.clearTimeout(gCurrentTimer);
	    }
	    gCurrentTimer = window.setTimeout(sync_pages,gUpdateInterval);
	}

    document.getElementById("OCRBuffer").onscroll = function() {
	    gCurrentFocus = 'ocr';
	    if(gCurrentTimer)
	    {
		window.clearTimeout(gCurrentTimer);
	    }
	    gCurrentTimer = window.setTimeout(sync_pages,gUpdateInterval);
	}

	let parsedUrlParams = parseURLParams(window.location.href);
	let scanId = parsedUrlParams['scanId'];
	document.getElementById("bookinfo").href = "/ShowMetadata.html?scanId="+scanId;
	let waitforocr = GetOCRTxt();
	
	Server.Get("/books/" + scanId+"/readerInfo.json", function(content) {
		let scanInfo = JSON.parse(content);
		
		scanInfo.pages.sort(function(a, b) {
			return(a.pageNumber - b.pageNumber);
		});
		for(let i = 0; i < scanInfo.pages.length; i++) {
			let svgCode = "<svg viewBox='0 0 " + 
				scanInfo.pages[i].width + " " + 
				scanInfo.pages[i].height + "'/>";

			let pageNode = document.createElement("div");
			pageNode.innerHTML = svgCode;
			pageNode.width_x = scanInfo.pages[i].width;
			pageNode.height_y = scanInfo.pages[i].height;
			pageNode.pageNumber = scanInfo.pages[i].pageNumber;
			pageNode.id = "uniqueDivId"+scanInfo.pages[i].pageNumber;
		    pageNode.ondblclick = function(){
		    console.log(this.pageNumber);
		    let page = document.getElementById("uniqueocrpage"+this.pageNumber);
		    let vspg = getVisiblePagesOCR();
		    if(vspg.indexOf(page)==-1)
		    {
			let fnc = this.onscroll;
			this.onscroll = null;
			page.scrollIntoView();
			if(window.history.replaceState)
			{
			    let newurl = window.location.search;
			    newurl+="#page"+this.pageNumber;
			    window.history.replaceState({},null,newurl);
			}
			this.onscroll = fnc;
		    }
		    };
			pageNode.classList.add("page");
			pageNode.classList.add("placeholder");
			let anchorNode = document.createElement("a");
			anchorNode.name = "page" + scanInfo.pages[i].pageNumber;
			anchorNode.id = "uniquePageNode"+scanInfo.pages[i].pageNumber;
			pageNode.insertBefore(anchorNode,pageNode.firstChild);
			document.querySelector(".reader").appendChild(pageNode);
			pageNode.scanFilePath = "/books/"+scanId+ "/" + scanInfo.pages[i].file.substr(scanInfo.pages[i].file.search("page"));
			pageNode.info = i;
		}

	    Server.Get("/getmetadata?scanId=" + scanId, function(content) {
		metadata = JSON.parse(content);
		    document.getElementById("biblio").innerHTML = metadata.bib;
		console.log(metadata.data.creators);
		if(metadata.data.creators.length>0)
		    document.title = metadata.data.creators[0].lastName+" ("+metadata.data.date+"). "+metadata.data.title;
	}, function() {
		// print some error message here
	});


		if(location.hash)
			location.hash = location.hash;

		updateAfterScroll();
	}, function() {
		document.getElementById("linklst").innerHTML = "Could not load images, sorry for that :(";
	});
	
	if(!parsedUrlParams.hasOwnProperty("query"))
		return;

	parsedUrlParams["query"] = decodeURIComponent(parsedUrlParams["query"]);
		
	Server.Get("/searchinbook?scanId=" + scanId + "&query=" + encodeURIComponent(parsedUrlParams["query"].replace(' ','+'))+"&fuzzyness="+encodeURIComponent(parsedUrlParams["fuzzyness"]), function(content) {
		let searchResults = JSON.parse(content);
		// temporary needed
		searchResults.pages = searchResults.books;

        if(searchResults.pages != undefined)
        {
		waitforocr.then(function(vwhatyever){
		
		if(searchResults.is_fuzzy === false)
		{
			
		  let pagesList = [];
		  let HighlightList = parsedUrlParams["query"].split("+");
		  for(let i = 0; i < searchResults.pages.length; i++)
		  	pagesList.push(searchResults.pages[i]);
		  pagesList = pagesList.filter((v, i, a) => a.indexOf(v) === i); 

		  let resultLinks = "";
		  for(let i = 0; i < pagesList.length; i++) {
			resultLinks = resultLinks + " " + "<a class='coollnklist' onclick='document.getElementById(\"uniqueocrpage"+pagesList[i]+"\").scrollIntoView();triggerDblclick(document.getElementById(\"uniqueocrpage"+pagesList[i]+"\"));changecolor(this);return true;' data-page='"+pagesList[i]+"'>" + pagesList[i] + "</a>";

			for(let y = 0; y < HighlightList.length; y++)
			{
				//TODO: Search Dieter with full search and get the results to be highlighted....
				console.log("show me what: "+HighlightList[y]);
				let strts = document.getElementById("uniqueocrpage"+pagesList[i]).innerHTML;
				document.getElementById("uniqueocrpage"+pagesList[i]).innerHTML=strts.replace(new RegExp('\\b'+HighlightList[y]+'\\b','gi'),' <mark>'+HighlightList[y]+'</mark> ');
			}
		  }

		
		  document.getElementById("linklst").innerHTML = resultLinks;
		}
		else
		{
		    let resultLinks = document.getElementById("linklst"); 
		  resultLinks.innerHTML = "";
		  for( let i = 0; i < searchResults.pages.length; i++)
		  {
			  let isok = document.getElementById("pageHitList"+searchResults.pages[i].page);
			  if(isok != null)
			  {
				  isok.innerHTML += ", "+searchResults.pages[i].word;
			  }
			  else
			  {
										   resultLinks.innerHTML = resultLinks.innerHTML + " " + "<a class='coollnklist' onclick='document.getElementById(\"uniqueocrpage"+searchResults.pages[i].page+"\").scrollIntoView();triggerDblclick(document.getElementById(\"uniqueocrpage"+searchResults.pages[i].page+"\"));changecolor(this);return true;' data-page='"+searchResults.pages[i].page+"'>" + searchResults.pages[i].page + "("
				  + "<span id='pageHitList"+searchResults.pages[i].page +"'>"					 
				  + searchResults.pages[i].word + "</span>);</a>";
			  }
			let thpage = document.getElementById("uniqueocrpage"+searchResults.pages[i].page);
			thpage.innerHTML=thpage.innerHTML.replace(new RegExp('(\\b'+searchResults.pages[i].word.replace(/[-[\]{}()*+?.,\\^$|#\s]/g, '\\$&')+'\\b)','gi'),'<mark>$1</mark>');
		  }
		}
		if(location.hash=="")
		    document.getElementsByClassName("coollnklist")[0].click();
	});
							      }}, function() {
		document.getElementById("linklst").innerHTML = "Could not load hit list :(";

	});


}


window.addEventListener("load", init, false);
let insideWindow = false;
function changecolor(x)
{
    x.setAttribute('style','cursor: pointer;color: purple !important');
    shownone(document.getElementById("SearchOpts"));
}

function shownone(x)
{
    if(x.classList.contains("currentoptioncontlay"))
    {
	document.getElementById("linklst").style["white-space"] = "nowrap";
	document.getElementById("upimg").style.display = "none";
	document.getElementById("downimg").style.display = "block";
	document.getElementById("divdrop").onclick = function(){showall(document.getElementById("SearchOpts"));};
	x.classList.replace("currentoptioncontlay","secondoptioncontlay");
    }
    x.scrollTop = 0;
    UpdateLinkPrev();
}

function showall(x)
{
    if(x.style.height != "40vh")
    {
	if(x.classList.contains("secondoptioncontlay"))
	    x.classList.replace("secondoptioncontlay","currentoptioncontlay");
	document.getElementById("linklst").style["white-space"] = "normal";
	document.getElementById("divdrop").onclick = function(){shownone(document.getElementById("SearchOpts"));};
	document.getElementById("upimg").style.display = "block";
	document.getElementById("downimg").style.display = "none";
    }
}

function SelectLastHit()
{
    SelectHit(false);
}

function SelectHit(direction)
{
    let curr = window.location.hash;
    let integer = parseInt(curr.substr(5));
    let lnks = document.getElementsByClassName("coollnklist");

    if(direction==false)
    {
	for(let i = lnks.length-1; i > -1; i--)
	{
	    if(parseInt(lnks[i].dataset.page) < integer)
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
	    if(parseInt(lnks[i].dataset.page) > integer)
	    {
		lnks[i].click();
		return;
	    }
	}
	//We are at the last hit so jump to the first one
	lnks[0].click();
    }
}


function SelectNextHit()
{
    SelectHit(true);
}

