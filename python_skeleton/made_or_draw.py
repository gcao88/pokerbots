
def made_or_draw(my_hand, my_board):
    """
    Returns a number representing whether the current hand is a made hand or a draw hand
    For now, 1 for a made hand, 0 for a draw 
    """
    hand = [0] * 13 
    board = [0] * 13 
    hand_suit = [0] * 4
    board_suit = [0] * 4
    mxBoard = 0
    for h in my_hand: 
        hand[card_number(h)] += 1
        hand_suit[card_suit(h)] += 1
    for b in my_board: 
        mxBoard = max(mxBoard, card_number(b))
        board[card_number(b)] += 1 
        board_suit[card_suit(b)] += 1

    for i in range(4): 
        if hand_suit[i] + board_suit[i] >= 5: 
            return 1 # Made flush

    gutShot = False  
    for i in range(9): 
        okay = True
        num = 0
        for j in range(5): 
            if hand[i + j] + board[i + j] < 1: 
                okay = False 
            else: 
                num += 1
        if num >= 4: 
            gutShot = True
        if okay: 
            return 1 # Made straight 

    for i in range(9): 
        okay2 = True
        for j in range(4): 
            if hand[j + i] + board[j + i] < 1: 
                okay2 = False 
        if okay2: 
            return 0 # Draw to a straight

    for i in range(4): 
        if hand_suit[i] + board_suit[i] == 4 and hand_suit[i] > 0: 
            return 0 # Draw to a flush 
    
    overs = 0 
    for i in range(mxBoard + 1, 13): 
        if hand[i] >= 2:
            return 1 # Overpair
        overs += hand[i]  

    if overs >= 2: 
        return 0 # Draw to overs  
    if gutShot: 
        return 0 
    return 1 #some sort of made hand, either high card or made pair+ 
    pass 

def card_number(card):
    return "23456789TJKQA".index(card[0])

def card_suit(card): 
    return "shcd".index(card[1]) 


# print(made_or_draw(["5s", "6s"], ["8s", "9s", "2s"]))