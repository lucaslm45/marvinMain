import json
from urllib import response
from flask import Flask, Response, request
from flask_cors import cross_origin

app = Flask(__name__)

@app.route("/")
@cross_origin()
def status():
    response = {
        "status": True,
        "info": "Hello! Welcome to Marvin API!"
    }
    return Response(json.dumps(response), status=200, mimetype='application/json')

@app.route('/api/coordinates', methods=['POST'])
@cross_origin()
def post_coordinates():
    body = request.json
    print(body['first_coordinate'])
    print(body['second_coordinate'])
    print(body['third_coordinate'])

    # Adicionar código que chama a função main com base nas coordenadas.

    return Response(status=200, mimetype='application/json')

