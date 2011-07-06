d={'I':1,'V':5,'X':10,'L':50,'C':100,'D':500,'M':1000}
def r(s):
	r=0
	for i in range(len(s)):
		a=d[s[i]]
		r+=a
		if(i>1):
			b=d[s[i-1]]
			if(b<a):r-=(b*2)

