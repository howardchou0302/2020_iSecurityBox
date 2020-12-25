import pymongo

def connect_mongodb(url, db_name="mydatabase", collection_name="mycollection"):
    myclient = pymongo.MongoClient(url)
    try:
        myclient.server_info()
    except Exception as e:
        raise e("Cannot connect to db!")

    # create database and collection
    mydb = myclient[db_name]
    mycol = mydb[collection_name]

    return mycol

def register_card(mongo_col, card_id, user_name):
    if not card_id or not isinstance(card_id, str):
        return { 'status': False, 'messsage': f"Invalid card ID {card_id}! Expected card ID <str>.", 'id': "" }
    elif not user_name or not isinstance(user_name, str):
        return { 'status': False, 'messsage': f"Invalid user name {user_name}! Expected user name <str>.", 'id': "" }

    data = {
        "card_id": card_id,
        "name": user_name
    }

    card = mongo_col.count_documents({ "card_id": card_id })
    if card != 0:
        return { 'status': False, 'message': f"Card id already exists!.", 'id': card_id }

    result = mongo_col.insert_one(data)
    if not result.inserted_id:
        return { 'status': False, 'message': "Error occurred while inserting card into database.", 'id': "" }
    
    return { 'status': True, 'message': "Success", 'id': result.inserted_id }
    
def query_card(mongo_col, card_id):
    if not card_id or not isinstance(card_id, str):
        return { 'status': False, 'message': "Warning: invalid card id! Expected <str>." }
    elif mongo_col.count_documents({ "card_id": card_id }) == 0:
        return { 'status': False, 'message': "Card not found!" }
    elif mongo_col.count_documents({ "card_id": card_id }) == 1:
        return { 'status': True, 'message': f"Card found! User: {mongo_col.find({ 'card_id': card_id })[0]['name']}." } # choose the only one user
    else:
        return { 'status': False, 'message': "Warning: multiple cards found. Make sure your code is correct." }

if __name__ == '__main__':
    mongo_url = "mongodb+srv://user:user@cluster0.plwco.mongodb.net/esys_final?retryWrites=true&w=majority"
    mongo_col = connect_mongodb(mongo_url)
    reg_result = register_card(mongo_col, card_id="12345678", user_name="Jian-Han")
    if not reg_result['status']:
        print(f"Cannot register card. Message: {reg_result['message']}")
    
    que_result = query_card(mongo_col, card_id="12345678")
    if not que_result['status']:
        print(f"Cannot find the expected card. Messgae: {que_result['message']}")
    
    print("Done")

