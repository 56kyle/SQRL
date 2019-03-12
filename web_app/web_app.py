
from flask import Flask, render_template, url_for

app = Flask(__name__)


@app.route('/')
def home():
    return render_template(url_for('home'))


@app.route('/connections')
def connections():
    return render_template(url_for('connections'))


@app.route('/connections/<conn>', methods=['GET', 'POST'])
def a_conn(conn):
    return render_template(url_for(conn))

@app.route('/admin')
def admin():
    return render_template(url_for('home'))

if __name__ == '__main__':
    app.run(debug=True)
