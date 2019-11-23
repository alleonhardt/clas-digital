let gGeneralJavascriptFile = 0;

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

function openFullscreen(elem) {
  if (elem.requestFullscreen) {
    elem.requestFullscreen();
  } else if (elem.mozRequestFullScreen) { /* Firefox */
    elem.mozRequestFullScreen();
  } else if (elem.webkitRequestFullscreen) { /* Chrome, Safari and Opera */
    elem.webkitRequestFullscreen();
  } else if (elem.msRequestFullscreen) { /* IE/Edge */
    elem.msRequestFullscreen();
  }
}

function hasFullscreen(elem) {
    let hasfull = false;
    if (elem.requestFullscreen) {
	hasfull = true;
  } else if (elem.mozRequestFullScreen) { /* Firefox */
	hasfull = true;
  } else if (elem.webkitRequestFullscreen) { /* Chrome, Safari and Opera */
	hasfull = true;
  } else if (elem.msRequestFullscreen) { /* IE/Edge */
	hasfull = true;
  }
    return hasfull;
}

/* Close fullscreen */
function closeFullscreen(elem) {
  if (document.exitFullscreen) {
    document.exitFullscreen();
  } else if (document.mozCancelFullScreen) { /* Firefox */
    document.mozCancelFullScreen();
  } else if (document.webkitExitFullscreen) { /* Chrome, Safari and Opera */
    document.webkitExitFullscreen();
  } else if (document.msExitFullscreen) { /* IE/Edge */
    document.msExitFullscreen();
  }
}
