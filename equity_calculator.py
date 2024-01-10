import itertools
import random
from tqdm import tqdm
from std_calculator import calculate_std_conf

def hand_strength(hand, board):
    all_cards = sorted(hand + board, reverse=True)
    return hand_value(max(itertools.combinations(all_cards, 5), key=hand_value))

def hand_value(hand):
    ranks = ['--23456789TJQKA'.index(r) for r, s in hand]
    ranks.sort(reverse=True)
    if len(set(r for r, s in hand)) == 5 and len(set(s for r, s in hand)) == 1 and max(ranks)-min(ranks) == 4: 
        return (8, ranks)  # straight flush
    elif len(set(ranks)) == 2: 
        return (7, ranks) if ranks.count(ranks[0]) in {1, 4} else (6, ranks)  # four of a kind or full house
    elif len(set(s for r, s in hand)) == 1: 
        return (5, ranks)  # flush
    elif len(set(ranks)) == 5 and max(ranks) - min(ranks) == 4: 
        return (4, ranks)  # straight
    elif len(set(ranks)) == 2 or len(set(ranks)) == 3: 
        return (3, ranks if ranks.count(ranks[2]) > 1 else ranks[::-1])  # three of a kind or two pair
    elif len(set(ranks)) == 4: 
        return (2, ranks if ranks.count(ranks[1]) > 1 else ranks[::-1])  # one pair
    else: 
        return (1, ranks)  # high card

def get_equity(h1, h2, board):
    deck = [r+s for r in '23456789TJQKA' for s in 'SHDC']
    for card in h1+h2+board:
        deck.remove(card)

    cnt, wins, losses, ties = 0, 0, 0, 0
    for b in itertools.combinations(deck, 5-len(board)):
        cnt += 1
        hand_1_strength = hand_strength(h1, list(b)+board)
        hand_2_strength = hand_strength(h2, list(b)+board)
        if hand_1_strength == hand_2_strength:
            ties += 1
        elif hand_1_strength > hand_2_strength:
            wins += 1
        else:
            losses += 1

    return (wins + 0.5*ties) / cnt

h1 = ['AC', '8D', '2C']
board = ['5H', '6C', '4D']

deck = [r+s for r in '23456789TJQKA' for s in 'SHDC']
for card in h1+board:
    deck.remove(card)

cnt, equities = 0, []
for _ in tqdm(range(100)):
    h2 = random.sample(deck, 2)
    equities.append(get_equity(h1, h2, board))
    cnt += 1

print(sum(equities) / len(equities))
print(calculate_std_conf(equities))