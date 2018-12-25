
/**
* Returns the week number for this date.  dowOffset is the day of week the week
* "starts" on for your locale - it can be from 0 to 6. If dowOffset is 1 (Monday),
* the week returned is the ISO 8601 week number.
* @param int dowOffset
* @return int
*/
function getWeek(date, dowOffset) {
    var newYear = new Date(date.getFullYear(),0,1);
    var day = newYear.getDay() - dowOffset; //the day of week the year begins on
    day = (day >= 0 ? day : day + 7);
    var daynum = Math.floor((date.getTime() - newYear.getTime() - (date.getTimezoneOffset()-newYear.getTimezoneOffset())*60000)/86400000) + 1;
    var weeknum;
    //if the year starts before the middle of a week
    if(day < 4) {
        weeknum = Math.floor((daynum+day-1)/7) + 1;
        if(weeknum > 52) {
            var nYear = new Date(date.getFullYear() + 1,0,1);
            var nday = nYear.getDay() - dowOffset;
            nday = nday >= 0 ? nday : nday + 7;
            /*if the next year starts before the middle of
            the week, it is week #1 of that year*/
            weeknum = nday < 4 ? 1 : 53;
        }
    }
    else {
        weeknum = Math.floor((daynum+day-1)/7);
    }
    return weeknum;
}

function roundToDay(date) {
    return new Date(date.getFullYear(), date.getMonth(), date.getDate())
}

function addDaysToDate(date, days) {
    var date = new Date(date);
    date.setDate(date.getDate() + days);
    return date;
}

function sameDay(date1, date2) {
    return date1.getFullYear() == date2.getFullYear() && date1.getMonth() == date2.getMonth() && date1.getDate() == date2.getDate()
}

function sameMonth(date1, date2) {
    return date1.getFullYear() == date2.getFullYear() && date1.getMonth() == date2.getMonth()
}

function nextWeek(date) {
    var d = date
    d.setTime(date.getTime() + (24*60*60*1000) * 7);
    return d
}

function previousWeek(date) {
    var d = date
    d.setTime(date.getTime() - (24*60*60*1000) * 7);
    return d
}

function nextMonth(date) {
    var d = date
    d.setMonth(date.getMonth() + 1);
    return d
}

function previousMonth(date) {
    var d = date
    d.setMonth(date.getMonth() - 1);
    return d
}

