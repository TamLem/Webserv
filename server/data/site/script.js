document.getElementById('file').addEventListener('change', function() {
    const uploadPath = '/uploads/' + this.value.substr(this.value.lastIndexOf('\\') + 1);
    console.log('upload path: ', uploadPath);
    document.getElementById('upload-form').setAttribute('action', uploadPath);
});

function uploadFile()
{
    console.log('uploading file...');
    var formData = new FormData();
    var file = document.getElementById('file').files[0];
    formData.append('file', file);
    axios.post('/uploads/' + file.name, formData, {
        headers: {
            'Content-Type': 'multipart/form-data'
        }
    })
    .then(function (response) {
        const msgElement = document.getElementById('upload-msg');
        msgElement.innerText = "File uploaded successfully";
        msgElement.style.display = 'block';
        msgElement.style.color = 'green';
        console.log(response.data);
    })
    .catch(function (error) {
        const msgElement = document.getElementById('upload-msg');
        msgElement.innerText = "File uploaded Failed";
        msgElement.style.display = 'block';
        msgElement.style.color = 'red';
        console.log(error);
    });
}

const uploadBtn = document.getElementById('upload-btn');
uploadBtn.addEventListener('click', function(e) {
    e.preventDefault();
    console.log('upload button clicked');
    var file = document.getElementById('file').files[0];
    if (!file) {
        const msgElement = document.getElementById('upload-msg');
        msgElement.innerText = "Please select a file to upload";
        msgElement.style.display = 'block';
        msgElement.style.color = 'red';
        return;
    }
    uploadFile();
});