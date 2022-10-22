import hashlib

testHash = ""
testPassword = ""

with open("result.txt", "r") as file:
    lines = file.readlines()
    for line in lines:
        testHash, testPassword = line.split(':')
        testPassword = testPassword[:-1:]

        hash = hashlib.sha3_256(testPassword.encode()).hexdigest()
        if hash == testHash:
            print("Verified!")
        else:
            print("Wrong password!")
            print(hash, testHash, testPassword)