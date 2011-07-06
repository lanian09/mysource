class Amazon:
	str = 20
	dex = 25
	vit = 20
	eng = 15

	def attack(self):
		return 'Jab!!!!'

	def exercise(self):
		self.str +=2
		self.dex +=3
		self.vit +=1

class Person:
	eyes=2
	nose=1
	mouth=1
	ears=2
	arms=2
	legs=2

	def eat(self):
		print '¾ä³È..'
	def sleep(self):
		print 'zzzzz..'
	def talk(self):
		print '#&$!@$%RT@#$'

class Student(Person):
	def study(self):
		print 'study!!!'

