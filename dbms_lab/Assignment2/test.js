var result = fetch('https://api.github.com', {
  method: 'post',
  headers: {
    'Accept': 'application/json',
    'Content-Type': 'application/json'
  },
  body: JSON.stringify({
    email: 'test@gmail.com',
    password: 'test',
  })
})

console.log(result);
