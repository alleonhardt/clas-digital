class User {
	//Static because it does not matter which user asks to end his session
	static DoLogout() {
		//Just tell the server to log the current user out, then reload the page
		 var now = new Date();
  		var time = now.getTime();
  		var expireTime = time - 10;
  		now.setTime(expireTime);
  		document.cookie = 'SESSID=;expires='+now.toGMTString()+';';
		window.location = window.location;
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
		specval.innerHTML = "<a id='homelink' href='/guest_index.html'>Home</a><a id='searchlink' href='/search'>Search</a><a id='administrationlink' class='classifiedContent' href='/private/admin/Administration.html' data-acc=4>Administration</a><a id='uploadbooklink' class='classifiedContent' href='/private/write/UploadBook.html' data-acc=2>Upload Books</a><a id='managebookslink' class='classifiedContent' href='/private/write/ManageBooks.html' data-acc=2>Manage Books</a><div class='topnav-right' id='loggedintopnav'><span id='LoggedInAs'>Logged in</span><button id='logoutBut' class='btn btn-outline-info' onclick='User.DoLogout()'>Logout</button></div>";

		console.log("Link: " + linkforactive);
		document.getElementById(linkforactive).classList.add("active");
		if(this.ServerDataObj.user == undefined)
		{
				let x = document.getElementById("loggedintopnav");
				x.innerHTML = '<form id ="LoginForm" class="form-inline my-2 my-lg-0" action="/login" method="post"><input id="EmailType" class="form-control mr-sm-2" type="email" name="email" placeholder="Email" aria-label="Email" required><input id="PasswordType" class="from-control mr-sm-2" type="password" name="password" placeholder="Password" aria-label="Password" required><input class="btn btn-outline-success my-2 my-sm-0" type="submit" value="Log In"/></form>';
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
