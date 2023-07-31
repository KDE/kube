

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

function roundToMinutes(date, delta) {
    var totalMinutes = date.getHours() * 60 +  date.getMinutes()
    //Round to nearest delta
    totalMinutes = Math.round(totalMinutes / delta) * delta
    var minutes = totalMinutes % 60
    var hours = (totalMinutes - minutes) / 60
    return new Date(date.getFullYear(), date.getMonth(), date.getDate(), hours, minutes, 0)
}

function addDaysToDate(date, days) {
    var date = new Date(date);
    date.setDate(date.getDate() + days);
    return date;
}

function addMinutesToDate(date, minutes) {
    return new Date(date.getTime() + minutes*60000);
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
    //FIXME, if you add a month to the 31.3 you will end up on the 5.1 because April has only 30 days. Wtf javascript
    d.setMonth(date.getMonth() + 1);
    return d
}

function previousMonth(date) {
    var d = date
    d.setMonth(date.getMonth() - 1);
    return d
}

function getFirstDayOfWeek(date) {
    //This works with negative days to get to the previous month
    //Date.getDate() is the day of the month, Date.getDay() is the day of the week
    // Examples:
    // 28 = 28 - (1 - 1)
    // 21 = 27 - (0 - 1)
    // 21 = 26 - (6 - 1)
    // 21 = 25 - (5 - 1)
    var offset = date.getDay() - Qt.locale().firstDayOfWeek;
    if (offset < 0) {
        offset = 7 + offset;
    }
    return new Date(date.getFullYear(), date.getMonth(), date.getDate() - offset)
}

function getFirstDayOfMonth(date) {
    var d = date
    d.setDate(1)
    return d
}

function daysSince(date1, date2) {
    var d1 = roundToDay(date1)
    var d2 = roundToDay(date2)
    const oneDay = 24 * 60 * 60 * 1000;
    return Math.round(Math.abs((d1.getTime() - d2.getTime()) / oneDay));
}

