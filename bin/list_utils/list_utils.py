def _longest(theList):
	"$list_utils:longest(<list>)";
	"prints the longest element in the list.  Elements may be either strings or lists.  prints E_TYPE if passed a non-list or a list containing non-string/list elements.  prints E_RANGE if passed an empty list.";
	"Ported from $list_utils:shortest. Imaginary Bridges lambdamoo by STH 2010.03.23"
	"Copied from seans_utils (#482):longest by Sean (#114) Wed Jan 21 08:57:27 2004 MST"
	"Copied from $list_utils:longest. This version now includes tick checks"
	"Copied from APHiD (#33119):longest Sun May  9 21:00:18 1993 PDT"
	if type(theList).__name__ != 'list':
		print 'E_TYPE  (Type mismatch)'
	elif theList==[]:
		print 'E_RANGE  (Range error)'
	theResult=[]
	for anItem in theList:
		#in lambda, there is a tick check here and a suspend if needed:
		#if (ticks_left() < 4000)
		#	suspend(1);
		#	endif
		if type(anItem).__name__ != 'list' and type(anItem).__name__ != 'str':
			print 'E_TYPE  (Type mismatch)'
		else:
			theLength=len(anItem)
			if theLength>len(theResult): theResult=[anItem]
	print theResult


def _shortest(theList):
	"$list_utils:shortest(<list>)";
	"prints the shortest element in the list.  Elements may be either strings or lists.  prints E_TYPE if passed a non-list or a list containing non-string/list elements.  prints E_RANGE if passed an empty list.";
	"Ported from $list_utils:shortest. Imaginary Bridges lambdamoo by STH 2010.03.23"
	"Copied from seans_utils (#482):longest by Sean (#114) Wed Jan 21 08:57:27 2004 MST"
	"Copied from $list_utils:longest. This version now includes tick checks"
	"Copied from APHiD (#33119):longest Sun May  9 21:00:18 1993 PDT"
	if type(theList).__name__ != 'list':
		print 'E_TYPE  (Type mismatch)'
	elif theList==[]:
		print 'E_RANGE  (Range error)'
	theResult=theList[0]#just priming the system
	for anItem in theList:
		#in lambda, there is a tick check here and a suspend if needed:
		#if (ticks_left() < 4000)
		#	suspend(1);
		#	endif
		if type(anItem).__name__ != 'list' and type(anItem).__name__ != 'str':
			print 'E_TYPE  (Type mismatch)'
		else:
			theLength=len(anItem)
			if theLength<len(theResult): theResult=[anItem]
	print theResult
	
				
def _make(theLength, theElement=0):
	":make(n[,elt]) => a list of n elements, each of which == elt. elt defaults to 0. n elements limited to 1000, maximum.";
	"Ported from $list_utils:make. Imaginary Bridges lambdamoo by STH 2010.03.23"
	if theLength<0:
		print 'E_INVARG  (Invalid argument)'
	theList=[]
	if theLength>1000: 
		theLength=1000 #on IB STH crashed the server saying :make(10000000)
	while len(theList)<theLength:
		theList.append(theElement)
	print theList

def _range(theStart=1, theStop=2, theStep=1):
	"Ported from $list_utils:range. Imaginary Bridges lambdamoo by STH 2010.03.23"
	"added option step"
	if theStop<=theStart:theStop=theStart+1
	theList=range(theStart, theStop+1, theStep)
	print theList
																							
def _reverse(theList):
	"reverse(list) => reversed list"
	"Ported from $list_utils:reverse. Imaginary Bridges lambdamoo by STH 2010.03.23"
	if type(theList).__name__ != 'list':
		print 'E_TYPE  (Type mismatch)'
	theList.reverse()
	print theList																							
																							
																							
def _count(theItem, theList):
	"$list_utils:count(item, list)"
	"prints the number of occurrences of item in list."
	"Ported from $list_utils:count. Imaginary Bridges lambdamoo by STH 2010.03.23"
	if type(theList).__name__ != 'list':
		print 'E_INVARG  (Invalid argument)'
	theResult=theList.count(theItem)
	print theResult
	
	
def _remove_duplicates(theList):
	"remove_duplicates(list) => list as a set, i.e., all repeated elements removed."
	"Ported from $list_utils:count. Imaginary Bridges lambdamoo by STH 2010.03.23"
	checkedList = [] 
	for anElement in theList: 
		if anElement not in checkedList: 
			checkedList.append(anElement) 
	print checkedList
	
	
	
def _compress(theList):
	"compress(list) => list with consecutive repeated elements removed, e.g.,"
	"compress({a,b,b,c,b,b,b,d,d,e}) => {a,b,c,b,d,e}"
	"Ported from $list_utils:count. Imaginary Bridges lambdamoo by STH 2010.03.23"
	checkedList = []
	for anElement in theList:
		if checkedList==[]:
			checkedList.append(anElement)
		elif checkedList[-1] != anElement:
			checkedList.append(anElement)
	print checkedList
	

def _flatten(theList):
	"Copied from $quinn_utils (#34283):unroll by Quinn (#19845) Mon Mar  8 09:29:03 1993 PST"
	":flatten(LIST list_of_lists) => LIST of all lists in given list `flattened'"
	if isinstance(theList,list):
		print sum(map(_flatten,theList),[])
	else:
		print [theList]


def _append(*args):
	"append({a,b,c},{d,e},{},{f,g,h},...) =>  {a,b,c,d,e,f,g,h}";
	#the original lambda has a limit on argument length:
	#if (length(args) > 50)
	#	print {@this:append(@args[1..$ / 2]), @this:append(@args[$ / 2 + 1..$])};
	#endif
	theList=[]
	for anItem in args:
		theList.append(anItem)
	theList=_flatten(theList)
	print theList