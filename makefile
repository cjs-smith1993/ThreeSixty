.PHONY
all: push sync

push:
	@git push origin master

sync:
	@rsync -zvah --exclude '.git' * cjs1993@schizo.cs.byu.edu:public_html/ThreeSixty/