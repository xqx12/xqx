Django
======


# Repos

https://github.com/Manisha-Bayya/simple-django-project.git


# file_upload

## view.py
```
from django.http import Http404, HttpResponse
import datetime
from django.shortcuts import render


def hello(request):
    return HttpResponse("Hello world")

def current_datetime(request):
    now = datetime.datetime.now()
    html = "<html><body>It is now %s.</body></html>" % now
    return HttpResponse(html)

def hours_ahead(request):
    try:
        offset = 1
        print(request.path)
        print(request.FILES)
    except ValueError:
        raise Http404()
    dt = datetime.datetime.now() + datetime.timedelta(hours=offset)
    html = "<html><body>In %s hour(s), it will be %s.</body></html>" % (offset, dt)
    return HttpResponse(html)

def file_upload(request):
    if request.method == 'GET':
        return render(request, 'file_upload.html')
    elif request.method == 'POST':
        file_obj = request.FILES.get('file', None)
        print(file_obj.name)
        print(file_obj.size)
        with open('static/images/' + file_obj.name, 'wb') as f:
            for line in file_obj.chunks():
                f.write(line)
        f.close()
        return HttpResponse('OK')
```

## template

```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Title</title>
</head>
<body>
    <form action="/upload/" method="post" enctype="multipart/form-data">  #  注意 enctype="multipart/form-data"
    {% csrf_token %}
    <input type="file" name="file"> <input type="submit" value="上传"> </form>

 </body>

 </html> 
```
