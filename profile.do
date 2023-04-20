// stata profile.do script
// set httpproxy only if running on compute node
// cja, 5 apr 23

quietly {
  if regexm("`c(hostname)'", "gl[0-9][0-9][0-9][0-9].arc-ts.umich.edu") {
    set httpproxyhost proxy1.arc-ts.umich.edu
    set httpproxyport 3128
    set httpproxy on
  }
  else {
    set httpproxy off
  }
}
