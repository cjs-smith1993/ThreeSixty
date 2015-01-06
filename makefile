push:
	@git push origin master

sync:
	@rsync -zvah --exclude '.git' * cjs1993@schizo.cs.byu.edu:public_html/ThreeSixty/

css: less/styles.less
	@lessc less/styles.less > css/styles.css
