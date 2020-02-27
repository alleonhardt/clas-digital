let global_timer = null;
let global_start = 0;
let global_count = 0;
let global_factor = 1;

function on_search_async(it,divide=1)
{
	let lst = document.getElementsByClassName("searchable_item");
	let num_items = Math.max(Math.floor((lst.length*global_factor)/50),20);
	global_factor++;

	if(global_start>=lst.length)
		return;
	it.value = it.value.toLowerCase();
	for(let i = 0; i < num_items; i++)
	{
		if((i+global_start)==lst.length)
		{
			global_start+=i;
			break;
		}
		let htm = lst[i+global_start].innerHTML;
		htm = htm.toLowerCase();
		if(htm.search(it.value) == -1)
			lst[i+global_start].parentNode.style.display = "none";
		else
		{
			lst[i+global_start].parentNode.style.display = "";
			global_count++;
			if(((i+global_start)%2 == 0) && (divide == 2))
			{
				++i;
			}
		}
	}
	if(it.value=="")
	{
		document.getElementById("filteropts").style.display="none";
	}
	else
	{
		document.getElementById("filteropts").innerHTML = "Filtering yielded "+global_count+" results<br>Searched "+Math.floor(global_start/divide)+"/"+Math.floor(lst.length/divide)+" entries";
		document.getElementById("filteropts").style.display="block";
	}
	global_start += num_items;
	global_timer = window.setTimeout(function(){on_search_async(it,divide)},150);
}

function on_search(it,divide=1)
{
	if(global_timer!=null)
	{
		window.clearTimeout(global_timer);
	}
	global_start = 0;
	global_count = 0;
	global_timer = null;
	global_factor = 1;

	on_search_async(it,divide);
}
