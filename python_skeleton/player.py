'''
Simple example pokerbot, written in Python.
'''
from skeleton.actions import FoldAction, CallAction, CheckAction, RaiseAction, BidAction
from skeleton.states import GameState, TerminalState, RoundState
from skeleton.states import NUM_ROUNDS, STARTING_STACK, BIG_BLIND, SMALL_BLIND
from skeleton.bot import Bot
from skeleton.runner import parse_args, run_bot
import csv
import random

class Player(Bot):
    '''
    A pokerbot.
    '''

    def __init__(self):
        '''
        Called when a new game starts. Called exactly once.

        Arguments:
        Nothing.

        Returns:
        Nothing.
        '''
        self.range_dict = {}
        self.card_num_list = []
        with open('preflop.csv', 'r', newline='') as file:
            csv_reader = csv.reader(file)
            i = 0
            for row in csv_reader:
                if i == 0:
                    self.card_num_list = row[1:]
                    i += 1
                else:
                    for j in range(1,len(row)):
                        small, big = row[j][1:-1].split(",")
                        self.range_dict[(str(self.card_num_list[i-1]), str(self.card_num_list[j-1]))] = (small, big)
                    i += 1
        pass

    def handle_new_round(self, game_state, round_state, active):
        '''
        Called when a new round starts. Called NUM_ROUNDS times.

        Arguments:
        game_state: the GameState object.
        round_state: the RoundState object.
        active: your player's index.

        Returns:
        Nothing.
        '''
        self.my_bankroll = game_state.bankroll  # the total number of chips you've gained or lost from the beginning of the game to the start of this round
        # game_clock = game_state.game_clock  # the total number of seconds your bot has left to play this game
        self.round_num = game_state.round_num  # the round number from 1 to NUM_ROUNDS
        # my_cards = round_state.hands[active]  # your cards
        self.big_blind = bool(active)  # True if you are the big blind
        pass

    def handle_round_over(self, game_state, terminal_state, active):
        '''
        Called when a round ends. Called NUM_ROUNDS times.

        Arguments:
        game_state: the GameState object.
        terminal_state: the TerminalState object.
        active: your player's index.

        Returns:
        Nothing.
        '''
        my_delta = terminal_state.deltas[active]  # your bankroll change from this round
        previous_state = terminal_state.previous_state  # RoundState before payoffs
        street = previous_state.street  # 0, 3, 4, or 5 representing when this round ended
        my_cards = previous_state.hands[active]  # your cards
        opp_cards = previous_state.hands[1-active]  # opponent's cards or [] if not revealed
        pass

    def get_action(self, game_state, round_state, active):
        '''
        Where the magic happens - your code should implement this function.
        Called any time the engine needs an action from your bot.

        Arguments:
        game_state: the GameState object.
        round_state: the RoundState object.
        active: your player's index.

        Returns:
        Your action.
        '''
        # May be useful, but you may choose to not use.
        legal_actions = round_state.legal_actions()  # the actions you are allowed to take
        street = round_state.street  # 0, 3, 4, or 5 representing pre-flop, flop, turn, or river respectively
        my_cards = round_state.hands[active]  # your cards
        board_cards = round_state.deck[:street]  # the board cards
        my_pip = round_state.pips[active]  # the number of chips you have contributed to the pot this round of betting
        opp_pip = round_state.pips[1-active]  # the number of chips your opponent has contributed to the pot this round of betting
        my_stack = round_state.stacks[active]  # the number of chips you have remaining
        opp_stack = round_state.stacks[1-active]  # the number of chips your opponent has remaining
        my_bid = round_state.bids[active]  # How much you bid previously (available only after auction)
        opp_bid = round_state.bids[1-active]  # How much opponent bid previously (available only after auction)
        continue_cost = opp_pip - my_pip  # the number of chips needed to stay in the pot
        my_contribution = STARTING_STACK - my_stack  # the number of chips you have contributed to the pot
        opp_contribution = STARTING_STACK - opp_stack  # the number of chips your opponent has contributed to the pot

        pot = STARTING_STACK - my_stack + STARTING_STACK - opp_stack

        if self.my_bankroll > 1.5 * (NUM_ROUNDS - self.round_num + 1):
            if CheckAction in legal_actions:
                return CheckAction()
            return FoldAction()
        if street == 0:
            # PRE-FLOP
            suited = my_cards[0][1] == my_cards[1][1]
            threshold = 400
            if self.card_num_list.index(my_cards[0][0]) > self.card_num_list.index(my_cards[1][0]):
                my_cards[0], my_cards[1] = my_cards[1], my_cards[0]
            if suited:
                threshold = self.range_dict[(my_cards[1][0], my_cards[0][0])][1 if self.big_blind else 0]
            else:
                threshold = self.range_dict[(my_cards[0][0], my_cards[1][0])][1 if self.big_blind else 0]
            threshold = int(threshold)

            if opp_pip > threshold:
                return FoldAction()
            else:
                # first bets as small blind
                if not self.big_blind and opp_pip == 2:
                    return RaiseAction(5)
                # ur small blind, they 3+ bet OR ur big blind
                else:
                    min_raise, max_raise = round_state.raise_bounds()
                    if threshold > 3*continue_cost:
                        return RaiseAction(min(max(3*opp_pip, min_raise), max_raise))
                    else:
                        return CallAction()
        elif BidAction in legal_actions:
            # AUCTION
            return BidAction(random.randint(3*pot, 4*pot))
        else:
            if CheckAction in legal_actions:
                return CheckAction()
            if CallAction in legal_actions:
                return CallAction()
            return FoldAction()

if __name__ == '__main__':
    run_bot(Player(), parse_args())
