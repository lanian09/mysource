
export _TEST_="ABC"

ifeq ($(_TEST_),)
AAA="NONE"
else
AAA=$(_TEST_)
endif
