build:
	docker image build -t thompatterson/picture-frame-server:0.3 .

run:
	docker container run --rm -it -p 12000:8080 -v /Users/thom/Development/PictureFrame/Server/data:/usr/data/videos --name picture-frame-server thompatterson/picture-frame-server:0.3

export:
	docker export -o test.tgz picture-frame-server

push:
	docker push thompatterson/picture-frame-server:0.3
