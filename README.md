# Compressor-Decompressor

This Compressor and Decompressor are used by Run-length Encoding.

=====Limitation and Performance:

Your soluton will not be tested against any ASCII text files that are larger than 200MB. 

In the testing machine, runtime memory is assumed to be always less than 20MB. Runtime memory consumption will be measured by valgrind massif with the option --pages-as-heap=yes, i.e., all the memory used by your program will be measured. Any solution that violates this memory requirement will receive zero points for that query test. Any solution that runs for more than 60 seconds on a machine with similar specification as wagner on a given text file will be killed, and will receive zero points for the tests for that file. We will use the time command and count both the user and system time as runtime measurement.

=====How to Use:

For example, suppose that the content of the input ASCII file called simple.txt is
aAAbbbBBBBcccccCCCCCCdDDeeeEEEE
Then
> ./rlencode simple.txt
aAAb[0]B[1]c[2]C[3]dDDe[0]E[1]

> ./rlencode simple.txt simple.rle
> ./rldecode simple.rle
aAAb[0]B[1]c[2]C[3]dDDe[0]E[1]


=====Encode Core Idea:

	This rlencode is mainly about two part,one argument and two arguments; I would like to outline 
	whole structure from big view.

	In order to deal with the big file, I set buff size as 4kb.However the key issue here is that how
	to deal the connection between the content of current buffer and the next buffer. I use the divide
	the buffer content as firstHalf and lastHalf to deal with it.I would show it detaily below. In 
	order to present them clearly, I use 3 variables:

	lastHalf: means the last consecutive sequence from last character in the buffer content
	firstHalf: means the remaining content except the lastHalf
	target: means the content we should processed... target = lastHalf + beforeHalf(normally)

		eg:  ...aaaccc|caabbcccc|cccddd...
			=>lastHalf:    ccc
			=>firstHalf:   caabb
			=>target:	   ccccaabb

	For example:
		Assume buffer size = 5
		===>plainText: aacbb	|bbbdd	|dddef	|ffff

		===>lastHalf: ""    bb       dd       f   ffff
		=>beforeHalf: aac         bbb     ddde    ""
		=>target:     aac    bbbbb    ddddde    ffffff

	As for how to deal with the count num, I list them below

	[0, 127]==> 		 1 _ _ _ _ _ _ _|
	[128, 16383]==>		 1 _ _ _ _ _ _ _ |1 _ _ _ _ _ _ _|
	[16384, 2097151]==>  1 _ _ _ _ _ _ _ |1 _ _ _ _ _ _ _|1 _ _ _ _ _ _ _|
	[2097152, 268435455] 1 _ _ _ _ _ _ _ |1 _ _ _ _ _ _ _|1 _ _ _ _ _ _ _|1 _ _ _ _ _ _ _|

===>One argument: when received one param, that means we should output debug info.
		The method of lastHalf and firstHalf can separate the content wisely, because it can ensure
		that there is no connection between the consecutive parts.

		When I get the target content, I process it and output to the screen when it reach 50 000
		characters in order to make it more efficiency...


===>Two argument: when received two params,that means we should write the binaryString or plainText
		to dest file.
		This method is almost same as one argument method.However, the last buffer content can be a
		litter tricky to deal with.In order to control the memory under 20M, I use several flags to 
		moniter them. When it reach max length, It would be ouput or write to file and then clear...

=====Decode Core Idea:

	This rldecode would be much more like rlencode, just the reverse version of encode;
	I would like to outline whole structure from big view.

	Similarly, I use the firstString and lastString to deal with connections between the current
	buffer content and next buffer content(they are same as rlencode!).

	For example(* denote ascii which highest bit is 1):
		Assume buffer size = 5
		===>plainText:   aabc*|**dde*|ffggd|*ab
		===>lastHalf:  ""   c*    e*     d   b
		=>beforeHalf:     aab  **dd  ffgg  *a
		=>target:      aab  c***dd e*ffgg d*a  b

	and then deal with the target string, that is traverse the target, if find a special
	character(*), I would convert it to bitset and then compute the actual count.

	For example: 
		target: aab*c
		1. I traverse target, if it's common character, just add it to output string
		2. If I find special character(*), I convert it into bitset, and then compute the
		   acutal num from it.Just 120, then I repeated the character for 120 times...
     
