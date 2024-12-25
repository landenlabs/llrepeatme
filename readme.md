
* * *

llRepatMe v1.2 (Dec 2024)
----------------------------------------------------------


Description
-----------

Repeat execution of command with optional sleep in between.

<pre>
 **Help banner:**
llRepeatMe v1.2 Dec 2024 - Repeat execution of target program

Author: Dennis Lang
https://lanenlabs.com

Usage:
    &lt;seconds> &lt;programToRun> [programArgs]...
   [-wait=&lt;seconds>] [-repeat=&lt;repeatCnt>] &lt;programToRun> [programArgs]...
   seconds to sleep between runs
   repeatCnt defaults to 10
   IF command string contains %d it is replaced with iteration count
Example:
   llrepeatme -repeat=100 netstat -rn
   llrepeatme -repeat=5 cmd /c "netstat -rn > net_\%02d.txt"
</pre>

[Top](#top)
