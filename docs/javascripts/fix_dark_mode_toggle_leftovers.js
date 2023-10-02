// We used to have a toggle dark/light mode.
// This created array with 2 values, for both palette options. The preferred
// color was kept in local storage, and some users were left with an invalid
// value. This is meant to remove this key, as it is no longer used.
//
//
// NOTE:
// This script can be removed, in 3 months from now (starting 1.12.23)
localStorage.removeItem("/latest/.__palette");
