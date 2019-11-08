class User {
    //Static because it does not matter which user asks to end his session
    static DoLogout() {
	//Just tell the server to log the current user out, then reload the page
	var now = new Date();
	var time = now.getTime();
	var expireTime = time - 10;
	now.setTime(expireTime);
	document.cookie = 'SESSID=;expires='+now.toGMTString()+';';
	window.location = "/";
    }

    static initialise(linkforactive) {
	if(linkforactive==undefined) linkforactive = 'searchlink';
	if (typeof(ServerDataObj) == 'undefined') {
	    this.ServerDataObj = {};
	}
	else
	    this.ServerDataObj = ServerDataObj;

	let vars = document.cookie;
	console.log("Document cookie: "+ document.cookie);
	let pos = vars.indexOf("SESSID=");
	if(pos!=-1)
	{
	    let numpos = vars.indexOf(";",pos+7);
	    let cookie = "";
	    if(numpos==-1)
		cookie = vars.substr(pos+7);
	    else
		cookie = vars.substr(pos+7,numpos);

	    console.log(cookie);
	    let pos2 = cookie.indexOf("::");
	    this.ServerDataObj.user = {};
	    this.ServerDataObj.user.email = cookie.substr(0,pos2);
	    this.ServerDataObj.user.access = parseInt(cookie.substr(pos2+2,cookie.indexOf(":::")));
	}

	let specval = document.getElementById("tpnav");
	specval.innerHTML = "<a id='homelink' href='/information/'>Information</a><a id='searchlink' href='/search'>Search</a><a id='administrationlink' class='classifiedContent' href='/private/admin/Administration.html' data-acc=4>Administration</a><a id='uploadbooklink' class='classifiedContent' href='/private/write/UploadBook.html' data-acc=2>Upload Books</a><a id='managebookslink' class='classifiedContent' href='/private/write/ManageBooks.html' data-acc=2>Manage Books</a><div class='topnav-right' id='loggedintopnav'><span id='LoggedInAs'>Logged in</span><button id='logoutBut' class='btn btn-outline-info' onclick='User.DoLogout()'>Logout</button></div>";

	console.log("Link: " + linkforactive);
	document.getElementById(linkforactive).classList.add("active");
	if(this.ServerDataObj.user == undefined)
	{
	    let x = document.getElementById("loggedintopnav");
	    x.innerHTML = '<form id ="LoginForm" class="form-inline my-2 my-lg-0" action="/login" method="post"><input id="EmailType" class="form-control mr-sm-2" type="email" name="email" placeholder="Email" aria-label="Email" required><input id="PasswordType" class="form-control mr-sm-2" type="password" name="password" placeholder="Password" aria-label="Password" required><input class="btn btn-outline-success my-2 my-sm-0" type="submit" value="Log In"/></form>';
	}
	else
	{
	    document.getElementById("LoggedInAs").innerHTML = "Logged in as <strong>"+this.ServerDataObj.user.email+"</strong>";

	    let list = document.getElementsByClassName("classifiedContent");

	    //Unlocks all the content that the user can access!
	    for(var i=0; i < list.length; i++)
	    {
		let y = list[i].dataset.acc;
		if((this.ServerDataObj.user.access&y)!=0)
		{
		    list[i].style.display="block";
		}
	    }
	}
    }
}

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

class GetBooksGlobalVars {

    static sync_pages()
    {
	let val;
	if(GetBooksGlobalVars.gCurrentFocus=='ocr')
	    val = GetBooksGlobalVars.getVisiblePagesOCR();
	else
	    val = GetBooksGlobalVars.getVisiblePages();
	GetBooksGlobalVars.triggerDblclick(val[0])
	UpdateLinkPrev();
	GetBooksGlobalVars.updateAfterScroll();
    }


    static init() {
	document.getElementById("srchbox").innerHTML = GetBooksGlobalVars.getParameterByName('query');
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
	    GetBooksGlobalVars.gCurrentFocus = 'page';
	    if(GetBooksGlobalVars.gCurrentTimer)
	    {
		window.clearTimeout(GetBooksGlobalVars.gCurrentTimer);
	    }
	    GetBooksGlobalVars.gCurrentTimer = window.setTimeout(GetBooksGlobalVars.sync_pages,400);
	}

	document.getElementById("OCRBuffer").onscroll = function() {
	    GetBooksGlobalVars.gCurrentFocus = 'ocr';
	    if(GetBooksGlobalVars.gCurrentTimer)
	    {
		window.clearTimeout(GetBooksGlobalVars.gCurrentTimer);
	    }
	    GetBooksGlobalVars.gCurrentTimer = window.setTimeout(GetBooksGlobalVars.sync_pages,400);
	}

	let parsedUrlParams = GetBooksGlobalVars.parseURLParams(window.location.href);
	let scanId = parsedUrlParams['scanId'];
	document.getElementById("bookinfo").href = "/ShowMetadata.html?scanId="+scanId;
	let waitforocr = GetBooksGlobalVars.GetOCRTxt();

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
		    let vspg = GetBooksGlobalVars.getVisiblePagesOCR();
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
		let metadata = JSON.parse(content);
		document.getElementById("biblio").innerHTML = metadata.bib;
		console.log(metadata.data.creators);
		if(metadata.data.creators.length>0)
		    document.title = metadata.data.creators[0].lastName+" ("+metadata.data.date+"). "+metadata.data.title;
	    }, function() {
		// print some error message here
	    });


	    if(location.hash)
		location.hash = location.hash;

	    GetBooksGlobalVars.updateAfterScroll();
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
			    resultLinks = resultLinks + " " + "<a class='coollnklist' onclick='document.getElementById(\"uniqueocrpage"+pagesList[i]+"\").scrollIntoView();GetBooksGlobalVars.triggerDblclick(document.getElementById(\"uniqueocrpage"+pagesList[i]+"\"));GetBooksGlobalVars.changecolor(this);return true;' data-page='"+pagesList[i]+"'>" + pagesList[i] + "</a>";

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
				resultLinks.innerHTML = resultLinks.innerHTML + " " + "<a class='coollnklist' onclick='document.getElementById(\"uniqueocrpage"+searchResults.pages[i].page+"\").scrollIntoView();GetBooksGlobalVars.triggerDblclick(document.getElementById(\"uniqueocrpage"+searchResults.pages[i].page+"\"));GetBooksGlobalVars.changecolor(this);return true;' data-page='"+searchResults.pages[i].page+"'>" + searchResults.pages[i].page + "("
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


    static changecolor(x)
    {
	x.setAttribute('style','cursor: pointer;color: purple !important');
	shownone(document.getElementById("SearchOpts"));
    }

    static shownone(x)
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

    static showall(x)
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

    static SelectLastHit()
    {
	SelectHit(false);
    }

    static SelectHit(direction)
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


    static SelectNextHit()
    {
	SelectHit(true);
    }

    static getVisiblePages()
    {
	let allPages = document.querySelectorAll("div.page");

	let visiblePages = new Array();
	for(let i = 0; i < allPages.length; i++) {
	    if(GetBooksGlobalVars.isElementInViewport(allPages[i])) {
		visiblePages.push(allPages[i]);
	    }
	}
	return(visiblePages);
    }

    static getVisiblePagesOCR()
    {
	let allPages = document.getElementsByClassName("ocrpage");
	let visiblePages = new Array();
	for(let i = 0; i < allPages.length; i++) {
	    if(GetBooksGlobalVars.isElementInViewport2(allPages[i])) {
		visiblePages.push(allPages[i]);
	    }
	}

	console.log(visiblePages[0]);
	return(visiblePages);
    }


    static updateAfterScroll() {
	let visiblePages = GetBooksGlobalVars.getVisiblePages();

	for(let i = 0; i < visiblePages.length; i++) {
	    if(visiblePages[i].classList.contains("placeholder"))
		GetBooksGlobalVars.pageLoader.queuePage(visiblePages[i]);
	}

	if(visiblePages.length > 0 && 
	    visiblePages[visiblePages.length - 1].nextElementSibling != null &&
	    visiblePages[visiblePages.length - 1].nextElementSibling.classList.contains("placeholder"))
	{
	    GetBooksGlobalVars.pageLoader.queuePage(visiblePages[visiblePages.length - 1].nextElementSibling);
	}
    }

    static isElementInViewport (el) {
	var rect = el.getBoundingClientRect();

	// it's definitely outside if bottom < 0
	if(rect.bottom <= 0)
	    return(false);
	if(rect.top > window.innerHeight)
	    return(false);

	return(true);
    }

    static isElementInViewport2(el) {
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

    static parseURLParams(url) {
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

    static doAdditionalSearch() {
	let parsedUrlParams = GetBooksGlobalVars.parseURLParams(window.location.href);
	window.open("/GetBooks?query="+document.getElementById("adSrch").value+"&scanId="+parsedUrlParams['scanId']+"&fuzzyness="+parsedUrlParams['fuzzyness']);

    }


    static GetOCRTxt() {
	return new Promise(function(resolve,reject){

	    let parsedUrlParams = GetBooksGlobalVars.parseURLParams(window.location.href);
	    Server.Get("/books/"+parsedUrlParams['scanId']+"/ocr.txt",function(content) {

		content = content.replace(new RegExp("<",'g'),"&lt;");
		content = content.replace(new RegExp(">",'g'),"&gt;");

		let RegSrch = /-----\s(\d+)\s.\s(\d+)\s-----/g;
		content = '<p data-page="1" id="uniqueocrpage0" class="ocrpage">' + content;
		content = content.replace(RegSrch,'</p><p data-page="$1" id="uniqueocrpage$1" class="ocrpage" ondblclick="GetBooksGlobalVars.SwitchToPage(this);"> Page $1 / $2 <br/>');
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

    static SwitchToPage(x)
    {
	let val = document.getElementById("uniqueDivId"+x.dataset.page);
	let vspg = GetBooksGlobalVars.getVisiblePages();
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


    static getParameterByName(name, url) {
	//COPYRIGHT TO https://stackoverflow.com/questions/901115/how-can-i-get-query-string-values-in-javascript
	if (!url) url = window.location.href;
	name = name.replace(/[\[\]]/g, '\\$&');
	var regex = new RegExp('[?&]' + name + '(=([^&#]*)|&|#|$)'),
	    results = regex.exec(url);
	if (!results) return null;
	if (!results[2]) return '';
	return decodeURIComponent(results[2].replace(/\+/g, ' '));
    }

    static UpdateLinkPrev()
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

    static triggerDblclick(x)
    {
	var event = new MouseEvent('dblclick', {
	    'view': window,
	    'bubbles': true,
	    'cancelable': true
	});
	x.dispatchEvent(event);
    }

    static DoFuzzyMatching(x)
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

    static doCompleteNewSearch()
    {
	let newurl = "/GetBooks.html?query="+document.getElementById("srchbox").value+"&scanId="+GetBooksGlobalVars.getParameterByName("scanId")+"&fuzzyness="+GetBooksGlobalVars.getParameterByName('fuzzyness');
	window.location = newurl;
    }

}
GetBooksGlobalVars.gCurrentTimer = null;
GetBooksGlobalVars.gCurrentFocus = 'ocr';
GetBooksGlobalVars.pageLoader = new PageLoader(); 


