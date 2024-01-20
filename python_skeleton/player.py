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
from equity_calculator import equity_vs_average_hand
from made_or_draw import made_or_draw

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

        # LINES
        # for now, flop vs turn vs river play the same
        # 8 lines for 8 different scenarios, see _identify_line
        # each line: 9 numbers, all relative to pot
        # 0-5: OOP, or if checked or tiny bet to us IP
        #   0: initial bet size
        #   1/2: thresholds if they re-raise
        #     1: re-raise under this amount
        #     2: call under this amount (fold over this amount)
        #   3/4: thresholds if they re-raise again
        # 6-9: IP and they bet a non-tiny amount
        #   6/7: initial thresholds
        #   8/9: thresholds if they re-raise
        # all numbers are relative to pot at the start of the round (so that responses to re-raises are more clear)
        self.lines_2hand = [
            [0, 0, 0, None, None, 0, 0, None, None],
            [0, 0, 0.5, None, None, 0.5, 0, None, None],
            [0, 0, 1.0, None, None, 1.0, 0, None, None],
            [0, 0, 400.0, None, None, 400.0, 0, None, None],
            [0, 0, 0, None, None, 0, 0, None, None],
            [0, 0, 0.5, None, None, 0.5, 0, None, None],
            [0, 0, 1.0, None, None, 1.0, 0, None, None],
            [0, 0, 400.0, None, None, 400.0, 0, None, None],
        ]
        self.lines_3hand = [
            [0.5, 0, 2.0, None, None, 0, 2.0, None, None],
            [0.5, 0, 2.0, None, None, 0, 2.0, None, None],
            [0.5, 0, 2.0, None, None, 0, 2.0, None, None],
            [0.5, 0, 2.0, None, None, 0, 2.0, None, None],
            [0.5, 0, 2.0, None, None, 0, 2.0, None, None],
            [0.5, 0, 2.0, None, None, 0, 2.0, None, None],
            [0.5, 0, 2.0, None, None, 0, 2.0, None, None],
            [0.5, 0, 2.0, None, None, 0, 2.0, None, None],
        ]
        self.RERAISE_MULTIPLIER = 2.5
        self.TINY_BET_THRESHOLD = 0.15
        self.AUCTION_AMOUNT_LOWER = 2.3
        self.AUCTION_AMOUNT_UPPER = 3.7

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

        # 0 if OOP and first action, 1 if IP and first action, 2 if OOP and it comes back to us, etc.
        self.street_action_counts = [None, None, None, 0, 0, 0]

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

        min_raise, max_raise = round_state.raise_bounds()
        starting_pot = 2*STARTING_STACK - my_stack - opp_stack + my_pip + opp_pip

        # if we win by folding every game, then check/fold
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

            if opp_pip > 1.5*threshold:
                return FoldAction()
            else:
                #### BACKDOOR FOR NEXT WEEK ####
                if opp_pip == "369":
                    return FoldAction()
                ################################
                
                min_raise, max_raise = round_state.raise_bounds()
                if threshold > 3*continue_cost:
                    return RaiseAction(min(max(3*opp_pip, min_raise), max_raise))
                else:
                    return CallAction()

        # ========
        # POSTFLOP
        # ========
        elif BidAction in legal_actions:
            # AUCTION
            return BidAction(random.randint(int(self.AUCTION_AMOUNT_LOWER*starting_pot), int(self.AUCTION_AMOUNT_UPPER*starting_pot)))
        else:
            # FLOP/TURN/RIVER

            #### EXPLOIT OF OLD BOT ####
            if RaiseAction in legal_actions:
                min_raise, max_raise = round_state.raise_bounds()
                return RaiseAction(max_raise)
            else:
                if CallAction in legal_actions:
                    return CallAction()
                if CheckAction in legal_actions:
                    return CheckAction()
                return FoldAction()
            ############################

            if my_bid > opp_bid:
                line = self.lines_3hand[self._identify_line(my_cards, board_cards)]
            else:
                line = self.lines_2hand[self._identify_line(my_cards, board_cards)]

            if self.street_action_counts[street] == 0:
                if opp_pip / starting_pot <= self.TINY_BET_THRESHOLD:
                    # OOP or IP and checked or small bet to us

                    # treat next actions as OOP
                    self.street_action_counts[street] = 2

                    if line[0] == 0:
                        if opp_pip > 0:
                            return CallAction()
                        else:
                            return CheckAction()
                    else: return RaiseAction(min(int(line[0] * starting_pot), max_raise))
                else:
                    # IP and bet to us
                    self.street_action_counts[street] = 3

                    return self._get_action_from_threshold(line[5], line[6], opp_pip, starting_pot, max_raise)
            else:
                if self.street_action_counts[street] == 2:
                    reraise_threshold, call_threshold = line[1], line[2]
                if self.street_action_counts[street] == 3:
                    reraise_threshold, call_threshold = line[7], line[8]
                if self.street_action_counts[street] == 4:
                    reraise_threshold, call_threshold = line[3], line[4]
                if self.street_action_counts[street] in [2,3,4]:
                    self.street_action_counts[street] += 2
                    return self._get_action_from_threshold(reraise_threshold, call_threshold, opp_pip, starting_pot, max_raise)
                else:
                    # edge case where somehow we aren't all in yet
                    print("womp womp")
                    return FoldAction()

    def _identify_line(self, hand, board):
        equity, conf_interval = equity_vs_average_hand(hand, 5-len(hand), board)
        hand_type = made_or_draw(hand, board)
        if hand_type == 1: # made hand
            if equity < 0.5: return 0
            elif equity < 0.65: return 1
            elif equity < 0.75: return 2
            elif equity <= 1: return 3
        elif hand_type == 0: # drawing
            if equity < 0.2: return 4
            elif equity < 0.5: return 5
            elif equity < 0.7: return 6
            elif equity <= 1: return 7

    def _get_action_from_threshold(self, reraise_threshold, call_threshold, opp_pip, starting_pot, max_raise):
        if opp_pip / starting_pot <= reraise_threshold:
            if 2 * self.RERAISE_MULTIPLIER * opp_pip > max_raise:
                return RaiseAction(max_raise)
            else:
                return RaiseAction(min(int(self.RERAISE_MULTIPLIER * opp_pip), max_raise))
        elif opp_pip / starting_pot <= call_threshold:
            return CallAction()
        else:
            return FoldAction()

if __name__ == '__main__':
    run_bot(Player(), parse_args())
