push:
	@git push origin master

sync:
	@rsync -zvah --exclude=.git --exclude=Lab1 * cjs1993@schizo.cs.byu.edu:public_html/ThreeSixty/

watch-css:
	@sass --watch --cache-location scss/cache/ scss:css

clean-css:
	@rm css/*
